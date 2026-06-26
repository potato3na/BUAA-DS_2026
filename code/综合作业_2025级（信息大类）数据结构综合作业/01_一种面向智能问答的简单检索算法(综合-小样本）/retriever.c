#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INIT_CAP 1024
#define EPS 1e-12

/* ---------- basic helpers ---------- */

static void *xmalloc(size_t n) {
    void *p = malloc(n ? n : 1);
    if (!p) exit(1);
    return p;
}

static void *xrealloc(void *p, size_t n) {
    void *q = realloc(p, n ? n : 1);
    if (!q) exit(1);
    return q;
}

static char *xstrdup2(const char *s) {
    size_t n = strlen(s);
    char *p = (char *)xmalloc(n + 1);
    memcpy(p, s, n + 1);
    return p;
}

static double dabs2(double x) {
    return x < 0 ? -x : x;
}

/* Avoid depending on -lm. Accuracy is enough for ranking/cosine. */
static double dsqrt2(double x) {
    if (x <= 0.0) return 0.0;
    double g = 1.0;
    while (g < x / g) g *= 2.0;
    for (int i = 0; i < 40; ++i) g = 0.5 * (g + x / g);
    return g;
}

static char *read_line_dynamic(FILE *fp) {
    int c;
    size_t len = 0, cap = 128;
    char *s = (char *)xmalloc(cap);

    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') break;
        if (len + 1 >= cap) {
            cap *= 2;
            s = (char *)xrealloc(s, cap);
        }
        s[len++] = (char)c;
    }

    if (c == EOF && len == 0) {
        free(s);
        return NULL;
    }

    if (len > 0 && s[len - 1] == '\r') len--;
    s[len] = '\0';
    return s;
}

static int is_blank_line(const char *s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        ++s;
    }
    return 1;
}

static char *read_nonempty_line(FILE *fp) {
    char *s;
    while ((s = read_line_dynamic(fp)) != NULL) {
        if (!is_blank_line(s)) return s;
        free(s);
    }
    return NULL;
}

static char *read_all_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return xstrdup2("");

    size_t len = 0, cap = 4096;
    char *buf = (char *)xmalloc(cap);
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            buf = (char *)xrealloc(buf, cap);
        }
        buf[len++] = (char)c;
    }
    fclose(fp);
    buf[len] = '\0';
    return buf;
}

/* ---------- stop list ---------- */

typedef struct {
    char **w;
    int n, cap;
} StopList;

static int cmp_str_ptr(const void *a, const void *b) {
    const char *pa = *(const char * const *)a;
    const char *pb = *(const char * const *)b;
    return strcmp(pa, pb);
}

static void stoplist_init(StopList *sl) {
    sl->n = 0;
    sl->cap = 256;
    sl->w = (char **)xmalloc(sizeof(char *) * sl->cap);
}

static void stoplist_push(StopList *sl, const char *s) {
    if (sl->n >= sl->cap) {
        sl->cap *= 2;
        sl->w = (char **)xrealloc(sl->w, sizeof(char *) * sl->cap);
    }
    sl->w[sl->n++] = xstrdup2(s);
}

static void load_stoplist(StopList *sl, const char *filename) {
    stoplist_init(sl);
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    char *line;
    while ((line = read_line_dynamic(fp)) != NULL) {
        if (!is_blank_line(line)) {
            char *p = line;
            while (*p && isspace((unsigned char)*p)) ++p;
            char *end = p + strlen(p);
            while (end > p && isspace((unsigned char)end[-1])) --end;
            *end = '\0';
            if (*p) stoplist_push(sl, p);
        }
        free(line);
    }
    fclose(fp);

    /* The file is said to be sorted; sorting again makes the program safer. */
    qsort(sl->w, sl->n, sizeof(char *), cmp_str_ptr);
}

static int is_stopword(const StopList *sl, const char *word) {
    int l = 0, r = sl->n - 1;
    while (l <= r) {
        int m = l + (r - l) / 2;
        int cmp = strcmp(word, sl->w[m]);
        if (cmp == 0) return 1;
        if (cmp < 0) r = m - 1;
        else l = m + 1;
    }
    return 0;
}

/* ---------- dictionary: word -> first-appearance id ---------- */

typedef struct {
    char *word;
    int id;
    int used;
} DictEntry;

typedef struct {
    DictEntry *tab;
    int cap;
    int size;
} Dict;

static unsigned long long hash_word(const char *s) {
    unsigned long long h = 1469598103934665603ULL;
    while (*s) {
        h ^= (unsigned char)(*s++);
        h *= 1099511628211ULL;
    }
    return h;
}

static void dict_init(Dict *d) {
    d->cap = 2048;
    d->size = 0;
    d->tab = (DictEntry *)calloc(d->cap, sizeof(DictEntry));
    if (!d->tab) exit(1);
}

static int dict_slot(DictEntry *tab, int cap, const char *word, int *found) {
    unsigned long long h = hash_word(word);
    int pos = (int)(h & (unsigned long long)(cap - 1));
    while (tab[pos].used) {
        int cmp = strcmp(tab[pos].word, word);
        if (cmp == 0) {
            *found = 1;
            return pos;
        }
        pos = (pos + 1) & (cap - 1);
    }
    *found = 0;
    return pos;
}

static void dict_rehash(Dict *d, int newCap) {
    DictEntry *old = d->tab;
    int oldCap = d->cap;

    d->tab = (DictEntry *)calloc(newCap, sizeof(DictEntry));
    if (!d->tab) exit(1);
    d->cap = newCap;

    for (int i = 0; i < oldCap; ++i) {
        if (old[i].used) {
            int found;
            int pos = dict_slot(d->tab, d->cap, old[i].word, &found);
            d->tab[pos] = old[i];
        }
    }
    free(old);
}

static int dict_find(const Dict *d, const char *word) {
    unsigned long long h = hash_word(word);
    int pos = (int)(h & (unsigned long long)(d->cap - 1));
    while (d->tab[pos].used) {
        if (strcmp(d->tab[pos].word, word) == 0) return d->tab[pos].id;
        pos = (pos + 1) & (d->cap - 1);
    }
    return -1;
}

static int dict_add(Dict *d, const char *word) {
    if ((d->size + 1) * 10 > d->cap * 7) dict_rehash(d, d->cap * 2);

    int found;
    int pos = dict_slot(d->tab, d->cap, word, &found);
    if (found) return d->tab[pos].id;

    d->tab[pos].used = 1;
    d->tab[pos].word = xstrdup2(word);
    d->tab[pos].id = d->size;
    d->size++;
    return d->tab[pos].id;
}

/* ---------- vector representation ---------- */

typedef struct {
    int id;
    int cnt;
} Pair;

typedef struct {
    Pair *p;
    int n;
    double semNorm;
    double raw[7];
    double st[7];
    double stNorm;
} Vec;

typedef struct {
    char *text;
    int index;
    Vec v;
} Sentence;

typedef struct {
    Sentence *a;
    int n, cap;
} SentArray;

static void sentarray_init(SentArray *sa) {
    sa->n = 0;
    sa->cap = 128;
    sa->a = (Sentence *)xmalloc(sizeof(Sentence) * sa->cap);
}

static void sentarray_push(SentArray *sa, Sentence s) {
    if (sa->n >= sa->cap) {
        sa->cap *= 2;
        sa->a = (Sentence *)xrealloc(sa->a, sizeof(Sentence) * sa->cap);
    }
    sa->a[sa->n++] = s;
}

typedef struct {
    int docMode;
    Dict *dict;
    int *freq;
    int freqCap;
    int *touched;
    int touchN, touchCap;
} SemCtx;

static void ensure_int_array(int **arr, int *cap, int need) {
    if (need <= *cap) return;
    int old = *cap;
    int nc = old ? old : 1024;
    while (nc < need) nc *= 2;
    *arr = (int *)xrealloc(*arr, sizeof(int) * nc);
    memset((*arr) + old, 0, sizeof(int) * (nc - old));
    *cap = nc;
}

static void touch_push(SemCtx *ctx, int id) {
    if (ctx->touchN >= ctx->touchCap) {
        ctx->touchCap = ctx->touchCap ? ctx->touchCap * 2 : 64;
        ctx->touched = (int *)xrealloc(ctx->touched, sizeof(int) * ctx->touchCap);
    }
    ctx->touched[ctx->touchN++] = id;
}

static void add_semantic_word(SemCtx *ctx, const char *word) {
    int id;
    if (ctx->docMode) id = dict_add(ctx->dict, word);
    else id = dict_find(ctx->dict, word);

    if (id < 0) return;             /* query-only unknown words do not enter semantic vector */
    ensure_int_array(&ctx->freq, &ctx->freqCap, id + 1);

    if (ctx->freq[id] == 0) touch_push(ctx, id);
    ctx->freq[id]++;
}

static int is_vowel_char(char c) {
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
}

static int is_sentence_delim(char c) {
    return c == '.' || c == '?' || c == '!' || c == '\n';
}

static int is_token_sep(char c) {
    unsigned char uc = (unsigned char)c;
    if (isspace(uc)) return 1;
    if (ispunct(uc) && c != '\'') return 1;
    return 0;
}

static void finish_token(char *buf, int len, const StopList *sl,
                         SemCtx *ctx, int *validWordN,
                         double *sumLen, double *sumSqLen, double *sumVowelRatio) {
    if (len <= 0) return;
    buf[len] = '\0';
    if (is_stopword(sl, buf)) return;

    (*validWordN)++;
    *sumLen += (double)len;
    *sumSqLen += (double)len * (double)len;

    int vowels = 0;
    for (int i = 0; i < len; ++i) {
        if (is_vowel_char(buf[i])) vowels++;
    }
    *sumVowelRatio += (double)vowels / (double)len;

    add_semantic_word(ctx, buf);
}

static void minmax_normalize(Vec *v) {
    double mn = v->raw[0], mx = v->raw[0];
    for (int i = 1; i < 7; ++i) {
        if (v->raw[i] < mn) mn = v->raw[i];
        if (v->raw[i] > mx) mx = v->raw[i];
    }

    double den = mx - mn;
    double ss = 0.0;
    if (dabs2(den) <= EPS) {
        for (int i = 0; i < 7; ++i) v->st[i] = 0.0;
        v->stNorm = 0.0;
        return;
    }

    for (int i = 0; i < 7; ++i) {
        v->st[i] = (v->raw[i] - mn) / den;
        ss += v->st[i] * v->st[i];
    }
    v->stNorm = dsqrt2(ss);
}

static Vec vectorize_text(const char *text, const StopList *sl, SemCtx *ctx) {
    Vec v;
    v.p = NULL;
    v.n = 0;
    v.semNorm = 0.0;
    for (int i = 0; i < 7; ++i) {
        v.raw[i] = 0.0;
        v.st[i] = 0.0;
    }
    v.stNorm = 0.0;

    int validChars = 0, punctN = 0, upperN = 0, digitN = 0;
    int validWordN = 0;
    double sumLen = 0.0, sumSqLen = 0.0, sumVowelRatio = 0.0;

    int tokenCap = 64, tokenLen = 0;
    char *token = (char *)xmalloc(tokenCap);

    for (int i = 0;; ++i) {
        char c = text[i];
        unsigned char uc = (unsigned char)c;

        if (c != '\0') {
            if (!isspace(uc)) validChars++;
            if (ispunct(uc)) punctN++;
            if (isupper(uc)) upperN++;
            if (isdigit(uc)) digitN++;
        }

        if (c == '\0' || is_token_sep(c)) {
            finish_token(token, tokenLen, sl, ctx, &validWordN,
                         &sumLen, &sumSqLen, &sumVowelRatio);
            tokenLen = 0;
            if (c == '\0') break;
        } else {
            if (tokenLen + 1 >= tokenCap) {
                tokenCap *= 2;
                token = (char *)xrealloc(token, tokenCap);
            }
            token[tokenLen++] = (char)tolower(uc);
        }
    }
    free(token);

    if (validWordN > 0) {
        double avg = sumLen / validWordN;
        v.raw[0] = avg;
        v.raw[1] = sumSqLen / validWordN - avg * avg;   /* sample uses division by n */
        if (v.raw[1] < 0 && v.raw[1] > -1e-9) v.raw[1] = 0.0;
        v.raw[2] = (double)validWordN;
        v.raw[3] = sumVowelRatio / validWordN;
    }
    if (validChars > 0) {
        v.raw[4] = (double)punctN / validChars;
        v.raw[5] = (double)upperN / validChars;
        v.raw[6] = (double)digitN / validChars;
    }

    minmax_normalize(&v);

    if (ctx->touchN > 0) {
        v.n = ctx->touchN;
        v.p = (Pair *)xmalloc(sizeof(Pair) * v.n);
        double ss = 0.0;
        for (int i = 0; i < ctx->touchN; ++i) {
            int id = ctx->touched[i];
            int cnt = ctx->freq[id];
            v.p[i].id = id;
            v.p[i].cnt = cnt;
            ss += (double)cnt * (double)cnt;
            ctx->freq[id] = 0;
        }
        v.semNorm = dsqrt2(ss);
        ctx->touchN = 0;
    }

    return v;
}

/* ---------- sentence splitting ---------- */

static char *trim_copy_range(const char *s, int l, int r) {
    while (l < r && isspace((unsigned char)s[l])) l++;
    while (r > l && isspace((unsigned char)s[r - 1])) r--;
    int n = r - l;
    if (n <= 0) return NULL;
    char *out = (char *)xmalloc(n + 1);
    memcpy(out, s + l, n);
    out[n] = '\0';
    return out;
}

static void build_document_sentences(const char *doc, const StopList *sl,
                                     Dict *dict, SentArray *sa) {
    SemCtx ctx;
    ctx.docMode = 1;
    ctx.dict = dict;
    ctx.freq = NULL;
    ctx.freqCap = 0;
    ctx.touched = NULL;
    ctx.touchN = 0;
    ctx.touchCap = 0;

    int start = 0;
    int idx = 0;
    int len = (int)strlen(doc);

    for (int i = 0; i < len; ++i) {
        if (is_sentence_delim(doc[i])) {
            char *sentText = trim_copy_range(doc, start, i + 1);
            if (sentText) {
                Sentence s;
                s.text = sentText;
                s.index = idx++;
                s.v = vectorize_text(sentText, sl, &ctx);
                sentarray_push(sa, s);
            }
            start = i + 1;
        }
    }

    if (start < len) {
        char *sentText = trim_copy_range(doc, start, len);
        if (sentText) {
            Sentence s;
            s.text = sentText;
            s.index = idx++;
            s.v = vectorize_text(sentText, sl, &ctx);
            sentarray_push(sa, s);
        }
    }

    free(ctx.freq);
    free(ctx.touched);
}

static Vec build_query_vec(const char *query, const StopList *sl, Dict *dict) {
    SemCtx ctx;
    ctx.docMode = 0;
    ctx.dict = dict;
    ctx.freqCap = dict->size > 0 ? dict->size : 1;
    ctx.freq = (int *)calloc(ctx.freqCap, sizeof(int));
    if (!ctx.freq) exit(1);
    ctx.touched = NULL;
    ctx.touchN = 0;
    ctx.touchCap = 0;

    Vec q = vectorize_text(query, sl, &ctx);
    free(ctx.freq);
    free(ctx.touched);
    return q;
}

/* ---------- similarity and top-K heap ---------- */

typedef struct {
    int idx;
    double score;
    const char *text;
} Result;

static double semantic_cosine(const Vec *q, const Vec *s, const int *qFreq) {
    if (q->semNorm <= EPS || s->semNorm <= EPS) return 0.0;
    double dot = 0.0;
    for (int i = 0; i < s->n; ++i) {
        int id = s->p[i].id;
        if (qFreq[id]) dot += (double)s->p[i].cnt * (double)qFreq[id];
    }
    if (dot == 0.0) return 0.0;
    return dot / (q->semNorm * s->semNorm);
}

static double structure_cosine(const Vec *q, const Vec *s) {
    if (q->stNorm <= EPS || s->stNorm <= EPS) return 0.0;
    double dot = 0.0;
    for (int i = 0; i < 7; ++i) dot += q->st[i] * s->st[i];
    return dot / (q->stNorm * s->stNorm);
}

static int better(Result a, Result b) {
    if (a.score > b.score + EPS) return 1;
    if (a.score < b.score - EPS) return 0;
    return a.idx < b.idx;
}

static int worse(Result a, Result b) {
    if (a.score < b.score - EPS) return 1;
    if (a.score > b.score + EPS) return 0;
    return a.idx > b.idx;
}

static void swap_result(Result *a, Result *b) {
    Result t = *a;
    *a = *b;
    *b = t;
}

static void heap_up(Result *h, int pos) {
    while (pos > 0) {
        int parent = (pos - 1) / 2;
        if (!worse(h[pos], h[parent])) break;
        swap_result(&h[pos], &h[parent]);
        pos = parent;
    }
}

static void heap_down(Result *h, int n, int pos) {
    while (1) {
        int l = pos * 2 + 1, r = l + 1, best = pos;
        if (l < n && worse(h[l], h[best])) best = l;
        if (r < n && worse(h[r], h[best])) best = r;
        if (best == pos) break;
        swap_result(&h[pos], &h[best]);
        pos = best;
    }
}

static int cmp_result_desc(const void *pa, const void *pb) {
    Result a = *(const Result *)pa;
    Result b = *(const Result *)pb;
    if (better(a, b)) return -1;
    if (better(b, a)) return 1;
    return 0;
}

static int *make_query_freq_dense(const Vec *q, int dictSize) {
    int *qFreq = (int *)calloc(dictSize > 0 ? dictSize : 1, sizeof(int));
    if (!qFreq) exit(1);
    for (int i = 0; i < q->n; ++i) qFreq[q->p[i].id] = q->p[i].cnt;
    return qFreq;
}

/* ---------- main ---------- */

int main(void) {
    char *query = read_line_dynamic(stdin);
    char *alphaLine = read_nonempty_line(stdin);
    char *kLine = read_nonempty_line(stdin);

    if (!query || !alphaLine || !kLine) {
        free(query);
        free(alphaLine);
        free(kLine);
        return 0;
    }

    double alpha = atof(alphaLine);
    int K = atoi(kLine);
    free(alphaLine);
    free(kLine);

    StopList sl;
    load_stoplist(&sl, "stopList.txt");

    Dict dict;
    dict_init(&dict);

    char *doc = read_all_file("document.txt");
    SentArray sa;
    sentarray_init(&sa);
    build_document_sentences(doc, &sl, &dict, &sa);

    Vec q = build_query_vec(query, &sl, &dict);
    int *qFreq = make_query_freq_dense(&q, dict.size);

    if (K > sa.n) K = sa.n;
    if (K < 0) K = 0;

    Result *heap = (Result *)xmalloc(sizeof(Result) * (K > 0 ? K : 1));
    int hn = 0;

    for (int i = 0; i < sa.n; ++i) {
        double s1 = semantic_cosine(&q, &sa.a[i].v, qFreq);
        double s2 = structure_cosine(&q, &sa.a[i].v);
        Result r;
        r.idx = sa.a[i].index;
        r.score = alpha * s1 + (1.0 - alpha) * s2;
        r.text = sa.a[i].text;

        if (K == 0) continue;
        if (hn < K) {
            heap[hn] = r;
            heap_up(heap, hn);
            hn++;
        } else if (better(r, heap[0])) {
            heap[0] = r;
            heap_down(heap, hn, 0);
        }
    }

    qsort(heap, hn, sizeof(Result), cmp_result_desc);
    for (int i = 0; i < hn; ++i) {
        printf("%s\n", heap[i].text);
    }

    free(heap);
    free(qFreq);
    free(query);
    free(doc);
    return 0;
}

