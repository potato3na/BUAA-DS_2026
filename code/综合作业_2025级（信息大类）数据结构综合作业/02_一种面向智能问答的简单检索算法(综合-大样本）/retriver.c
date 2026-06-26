#ifdef __GNUC__
#pragma GCC optimize("O3")
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ZERO_EPS 1e-12
#define STOP_HASH_CAP 4096
#define LOCAL_HASH_CAP 256
#define INV_LEN_CAP 1024

#define F_SPACE 1
#define F_PUNCT 2
#define F_UPPER 4
#define F_DIGIT 8
#define F_SEP 16
#define F_SENT 32
#define F_VOWEL 64

/*
 * 极限优化版思路：
 *
 * 题目只有 1 个 query，所以不需要真的保存每个句子的完整语义稀疏向量。
 * 直接先解析 query，建立 query 词频哈希表。
 * 扫 document 时：
 *   1. 正常建立全局词典 id，用于统计每个句子的 TF 范数。
 *   2. 同时如果当前词在 query 中出现，直接给当前句子的 semantic dot 累加 qcnt。
 *   3. 每个句子只保存：原文指针+长度、semantic dot、semantic norm、结构向量。
 *
 * 这样最后打分时不再扫描句子的词表，速度更快，内存也更低。
 */

/* ---------- fast char table ---------- */

static unsigned char T_SPACE[256], T_PUNCT[256], T_UPPER[256], T_DIGIT[256];
static unsigned char T_LOWER[256], T_SEP[256], T_VOWEL[256], T_SENT[256];
static unsigned char T_FLAG[256];
static double T_INV_LEN[INV_LEN_CAP];

static void init_char_table(void) {
    int i;
    for (i = 0; i < 256; ++i) {
        T_LOWER[i] = (unsigned char)i;
    }

    /* ASCII whitespace: space, tab, LF, VT, FF, CR */
    T_SPACE[32] = 1;
    T_SPACE[9] = 1;
    T_SPACE[10] = 1;
    T_SPACE[11] = 1;
    T_SPACE[12] = 1;
    T_SPACE[13] = 1;

    for (i = 48; i <= 57; ++i) T_DIGIT[i] = 1;
    for (i = 65; i <= 90; ++i) {
        T_UPPER[i] = 1;
        T_LOWER[i] = (unsigned char)(i + 32);
    }

    /* ASCII punctuation: same range behavior as ispunct for normal English text */
    for (i = 33; i <= 47; ++i) T_PUNCT[i] = 1;
    for (i = 58; i <= 64; ++i) T_PUNCT[i] = 1;
    for (i = 91; i <= 96; ++i) T_PUNCT[i] = 1;
    for (i = 123; i <= 126; ++i) T_PUNCT[i] = 1;

    for (i = 0; i < 256; ++i) {
        T_SEP[i] = (unsigned char)(T_SPACE[i] || (T_PUNCT[i] && i != 39));
    }

    T_VOWEL[97] = 1;
    T_VOWEL[101] = 1;
    T_VOWEL[105] = 1;
    T_VOWEL[111] = 1;
    T_VOWEL[117] = 1;

    T_SENT[46] = 1;
    T_SENT[63] = 1;
    T_SENT[33] = 1;
    T_SENT[10] = 1;

    for (i = 0; i < 256; ++i) {
        T_FLAG[i] = (unsigned char)(
            (T_SPACE[i] ? F_SPACE : 0) |
            (T_PUNCT[i] ? F_PUNCT : 0) |
            (T_UPPER[i] ? F_UPPER : 0) |
            (T_DIGIT[i] ? F_DIGIT : 0) |
            (T_SEP[i] ? F_SEP : 0) |
            (T_SENT[i] ? F_SENT : 0) |
            (T_VOWEL[i] ? F_VOWEL : 0));
    }

    T_INV_LEN[0] = 0.0;
    for (i = 1; i < INV_LEN_CAP; ++i) T_INV_LEN[i] = 1.0 / (double)i;
}

/* ---------- basic helpers ---------- */

static char *xstrndup2(const char *s, int n) {
    char *p = (char *)malloc((size_t)n + 1);
    memcpy(p, s, (size_t)n);
    p[n] = '\0';
    return p;
}

static char *g_tok_buf = NULL;
static int g_tok_cap = 0;

static char *get_token_buf(int need) {
    if (need <= g_tok_cap) return g_tok_buf;
    int nc = g_tok_cap ? g_tok_cap : 128;
    while (nc < need) nc <<= 1;
    g_tok_buf = (char *)realloc(g_tok_buf, nc);
    g_tok_cap = nc;
    return g_tok_buf;
}

static char *xstrdup2(const char *s) {
    return xstrndup2(s, (int)strlen(s));
}

/* 快速 sqrt：避免依赖 -lm；bit 初值 + Newton，通常比纯 Newton 快很多。 */
static double dsqrt2(double x) {
    if (x <= 0.0) return 0.0;
#ifdef __GNUC__
    return __builtin_sqrt(x);
#else
    union { double d; unsigned long long i; } u;
    u.d = x;
    u.i = (u.i >> 1) + 0x1ff8000000000000ULL;
    double g = u.d;
    for (int k = 0; k < 5; ++k) g = 0.5 * (g + x / g);
    return g;
#endif
}

static char *read_all_file(const char *filename, long *outLen) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        *outLen = 0;
        return xstrdup2("");
    }

    fseek(fp, 0, SEEK_END);
    long n = ftell(fp);
    if (n < 0) n = 0;
    rewind(fp);

    char *buf = (char *)malloc((size_t)n + 1);
    size_t got = fread(buf, 1, (size_t)n, fp);
    fclose(fp);
    buf[got] = '\0';
    *outLen = (long)got;
    return buf;
}

static char *read_line_stdin(void) {
    int c, n = 0, cap = 128;
    char *s = (char *)malloc(cap);
    while ((c = getchar()) != EOF) {
        if (c == '\n') break;
        if (n + 1 >= cap) {
            cap <<= 1;
            s = (char *)realloc(s, cap);
        }
        s[n++] = (char)c;
    }
    if (c == EOF && n == 0) {
        free(s);
        return NULL;
    }
    if (n > 0 && s[n - 1] == '\r') --n;
    s[n] = '\0';
    return s;
}

static int is_blank_line(const char *s) {
    while (*s) {
        if (!(T_FLAG[(unsigned char)*s] & F_SPACE)) return 0;
        ++s;
    }
    return 1;
}

static char *read_nonblank_stdin(void) {
    char *s;
    while ((s = read_line_stdin()) != NULL) {
        if (!is_blank_line(s)) return s;
        free(s);
    }
    return NULL;
}

/* ---------- hash ---------- */

#define HASH_INIT 1469598103934665603ULL
#define HASH_STEP(h, c) (((h) ^ (unsigned long long)(unsigned char)(c)) * 1099511628211ULL)

/* ---------- stopword hash set ---------- */

typedef struct {
    char *word;
    unsigned long long h;
    int len;
    unsigned char used;
} WordSlot;

typedef struct {
    WordSlot *tab;
    int cap, size;
    int minLen, maxLen;
    char *storage;
} WordSet;

static void wordset_init(WordSet *ws) {
    ws->cap = STOP_HASH_CAP;
    ws->size = 0;
    ws->minLen = 1000000;
    ws->maxLen = 0;
    ws->storage = NULL;
    ws->tab = (WordSlot *)calloc(ws->cap, sizeof(WordSlot));
}

static int wordset_find_slot(WordSlot *tab, int cap, const char *word, int len, unsigned long long h, int *found) {
    int pos = (int)(h & (unsigned long long)(cap - 1));
    while (tab[pos].used) {
        if (tab[pos].h == h && tab[pos].len == len && memcmp(tab[pos].word, word, (size_t)len) == 0) {
            *found = 1;
            return pos;
        }
        pos = (pos + 1) & (cap - 1);
    }
    *found = 0;
    return pos;
}

static void wordset_rehash(WordSet *ws, int newCap) {
    WordSlot *old = ws->tab;
    int oldCap = ws->cap;
    ws->tab = (WordSlot *)calloc(newCap, sizeof(WordSlot));
    ws->cap = newCap;
    for (int i = 0; i < oldCap; ++i) {
        if (old[i].used) {
            int found;
            int pos = wordset_find_slot(ws->tab, ws->cap, old[i].word, old[i].len, old[i].h, &found);
            ws->tab[pos] = old[i];
        }
    }
    free(old);
}

static unsigned long long hash_bytes(const char *s, int len) {
    unsigned long long h = HASH_INIT;
    for (int i = 0; i < len; ++i) {
        h = HASH_STEP(h, s[i]);
    }
    return h ? h : 1ULL;
}

static int lower_span_equal(const char *span, const char *lowerWord, int len) {
    for (int i = 0; i < len; ++i) {
        if ((char)T_LOWER[(unsigned char)span[i]] != lowerWord[i]) return 0;
    }
    return 1;
}

static void wordset_add_ref(WordSet *ws, char *word, int len) {
    if ((ws->size + 1) * 2 > ws->cap) wordset_rehash(ws, ws->cap << 1);
    unsigned long long h = hash_bytes(word, len);
    int found;
    int pos = wordset_find_slot(ws->tab, ws->cap, word, len, h, &found);
    if (found) return;
    ws->tab[pos].used = 1;
    ws->tab[pos].h = h;
    ws->tab[pos].len = len;
    ws->tab[pos].word = word;
    if (len < ws->minLen) ws->minLen = len;
    if (len > ws->maxLen) ws->maxLen = len;
    ws->size++;
}

static int wordset_contains_len(const WordSet *ws, const char *word, int len, unsigned long long h) {
    if (len < ws->minLen || len > ws->maxLen) return 0;
    int pos = (int)(h & (unsigned long long)(ws->cap - 1));
    while (ws->tab[pos].used) {
        if (ws->tab[pos].h == h && ws->tab[pos].len == len && memcmp(ws->tab[pos].word, word, (size_t)len) == 0) return 1;
        pos = (pos + 1) & (ws->cap - 1);
    }
    return 0;
}

static int wordset_contains_lower_span(const WordSet *ws, const char *span, int len, unsigned long long h) {
    if (len < ws->minLen || len > ws->maxLen) return 0;
    int pos = (int)(h & (unsigned long long)(ws->cap - 1));
    while (ws->tab[pos].used) {
        WordSlot *slot = &ws->tab[pos];
        if (slot->h == h && slot->len == len && lower_span_equal(span, slot->word, len)) return 1;
        pos = (pos + 1) & (ws->cap - 1);
    }
    return 0;
}

static void load_stopwords(WordSet *stop, const char *filename) {
    wordset_init(stop);
    long len;
    char *buf = read_all_file(filename, &len);

    int l = 0;
    for (int i = 0; i <= len; ++i) {
        if (i == len || buf[i] == '\n') {
            int a = l, b = i;
            if (b > a && buf[b - 1] == '\r') --b;
            while (a < b && (T_FLAG[(unsigned char)buf[a]] & F_SPACE)) ++a;
            while (b > a && (T_FLAG[(unsigned char)buf[b - 1]] & F_SPACE)) --b;
            if (a < b) {
                for (int j = a; j < b; ++j) buf[j] = (char)T_LOWER[(unsigned char)buf[j]];
                buf[b] = '\0';
                wordset_add_ref(stop, buf + a, b - a);
            }
            l = i + 1;
        }
    }
    stop->storage = buf;
}

/* ---------- query word set ---------- */

typedef struct {
    char *word;
    unsigned long long h;
    int len;
    int cnt;
    unsigned char appeared;
    unsigned char used;
} QSlot;

typedef struct {
    QSlot *tab;
    unsigned long long mask;
    int cap, size;
} QSet;

static void qset_init(QSet *qs) {
    qs->cap = 256;
    qs->size = 0;
    qs->mask = 0;
    qs->tab = (QSlot *)calloc(qs->cap, sizeof(QSlot));
}

static int qset_find_slot(QSlot *tab, int cap, const char *word, int len, unsigned long long h, int *found) {
    int pos = (int)(h & (unsigned long long)(cap - 1));
    while (tab[pos].used) {
        if (tab[pos].h == h && tab[pos].len == len && memcmp(tab[pos].word, word, (size_t)len) == 0) {
            *found = 1;
            return pos;
        }
        pos = (pos + 1) & (cap - 1);
    }
    *found = 0;
    return pos;
}

static void qset_rehash(QSet *qs, int newCap) {
    QSlot *old = qs->tab;
    int oldCap = qs->cap;
    qs->tab = (QSlot *)calloc(newCap, sizeof(QSlot));
    qs->cap = newCap;
    for (int i = 0; i < oldCap; ++i) {
        if (old[i].used) {
            int found;
            int pos = qset_find_slot(qs->tab, qs->cap, old[i].word, old[i].len, old[i].h, &found);
            qs->tab[pos] = old[i];
        }
    }
    free(old);
}

static void qset_add(QSet *qs, const char *word, int len, unsigned long long h) {
    if ((qs->size + 1) * 2 > qs->cap) qset_rehash(qs, qs->cap << 1);
    int found;
    int pos = qset_find_slot(qs->tab, qs->cap, word, len, h, &found);
    if (found) {
        qs->tab[pos].cnt++;
        return;
    }
    qs->tab[pos].used = 1;
    qs->tab[pos].h = h;
    qs->tab[pos].len = len;
    qs->tab[pos].word = xstrndup2(word, len);
    qs->tab[pos].cnt = 1;
    qs->tab[pos].appeared = 0;
    qs->mask |= 1ULL << (h & 63);
    qs->size++;
}

static QSlot *qset_get_lower_span(QSet *qs, const char *span, int len, unsigned long long h) {
    if ((qs->mask & (1ULL << (h & 63))) == 0) return NULL;
    int pos = (int)(h & (unsigned long long)(qs->cap - 1));
    while (qs->tab[pos].used) {
        QSlot *slot = &qs->tab[pos];
        if (slot->h == h && slot->len == len && lower_span_equal(span, slot->word, len)) return slot;
        pos = (pos + 1) & (qs->cap - 1);
    }
    return NULL;
}

/* ---------- per-sentence word counts ---------- */

typedef struct {
    unsigned long long h;
    int len;
    int cnt;
    unsigned int stamp;
} LocalSlot;

typedef struct {
    LocalSlot *tab;
    int cap, size;
    unsigned int stamp;
} LocalWords;

static void localwords_init(LocalWords *lw) {
    lw->cap = LOCAL_HASH_CAP;
    lw->size = 0;
    lw->stamp = 1;
    lw->tab = (LocalSlot *)calloc(lw->cap, sizeof(LocalSlot));
}

static void localwords_reset(LocalWords *lw) {
    lw->size = 0;
    if (++lw->stamp == 0) {
        memset(lw->tab, 0, sizeof(LocalSlot) * lw->cap);
        lw->stamp = 1;
    }
}

static void localwords_rehash(LocalWords *lw, int newCap) {
    LocalSlot *old = lw->tab;
    int oldCap = lw->cap;
    unsigned int stamp = lw->stamp;
    lw->tab = (LocalSlot *)calloc(newCap, sizeof(LocalSlot));
    lw->cap = newCap;
    int n = 0;
    for (int i = 0; i < oldCap; ++i) {
        if (old[i].stamp == stamp) {
            int pos = (int)(old[i].h & (unsigned long long)(newCap - 1));
            while (lw->tab[pos].stamp == stamp) pos = (pos + 1) & (newCap - 1);
            lw->tab[pos] = old[i];
            ++n;
        }
    }
    lw->size = n;
    free(old);
}

static void localwords_add(LocalWords *lw, int len, unsigned long long h, double *sumSq) {
    if ((lw->size + 1) * 2 > lw->cap) localwords_rehash(lw, lw->cap << 1);

    int pos = (int)(h & (unsigned long long)(lw->cap - 1));
    while (lw->tab[pos].stamp == lw->stamp) {
        LocalSlot *slot = &lw->tab[pos];
        if (slot->h == h && slot->len == len) {
            *sumSq += (double)(slot->cnt * 2 + 1);
            slot->cnt++;
            return;
        }
        pos = (pos + 1) & (lw->cap - 1);
    }

    lw->tab[pos].stamp = lw->stamp;
    lw->tab[pos].h = h;
    lw->tab[pos].len = len;
    lw->tab[pos].cnt = 1;
    ++lw->size;
    *sumSq += 1.0;
}

/* ---------- vectors and sentence data ---------- */

typedef struct {
    QSet *qset;
    LocalWords words;
    double qSemSq;
} WorkCtx;

typedef struct {
    const char *text;
    int len;
    int idx;
    double semBase;
    double stScore;
    double score;
} Sentence;

typedef struct {
    Sentence *a;
    int n, cap;
} SentArray;

typedef struct {
    QSet qset;
    double raw[7];
    double st[7];
    double stInvNorm;
    double semInvNorm;
} Query;

static void sentarray_init(SentArray *sa) {
    sa->n = 0;
    sa->cap = 256;
    sa->a = (Sentence *)malloc(sizeof(Sentence) * sa->cap);
}

static void sentarray_push(SentArray *sa, Sentence s) {
    if (sa->n == sa->cap) {
        sa->cap <<= 1;
        sa->a = (Sentence *)realloc(sa->a, sizeof(Sentence) * sa->cap);
    }
    sa->a[sa->n++] = s;
}

static void normalize7(const double raw[7], double out[7], double *invNorm) {
    double mn = raw[0], mx = raw[0];
    for (int i = 1; i < 7; ++i) {
        if (raw[i] < mn) mn = raw[i];
        if (raw[i] > mx) mx = raw[i];
    }
    double den = mx - mn;
    if (den <= ZERO_EPS) {
        for (int i = 0; i < 7; ++i) out[i] = 0.0;
        *invNorm = 0.0;
        return;
    }
    double ss = 0.0;
    for (int i = 0; i < 7; ++i) {
        out[i] = (raw[i] - mn) / den;
        ss += out[i] * out[i];
    }
    *invNorm = ss > 0.0 ? 1.0 / dsqrt2(ss) : 0.0;
}

static double structure_score_vals(double r0, double r1, double r2, double r3,
                                   double r4, double r5, double r6, const Query *q) {
    if (q->stInvNorm <= 0.0) return 0.0;
    double mn = r0, mx = r0;
    if (r1 < mn) mn = r1;
    if (r1 > mx) mx = r1;
    if (r2 < mn) mn = r2;
    if (r2 > mx) mx = r2;
    if (r3 < mn) mn = r3;
    if (r3 > mx) mx = r3;
    if (r4 < mn) mn = r4;
    if (r4 > mx) mx = r4;
    if (r5 < mn) mn = r5;
    if (r5 > mx) mx = r5;
    if (r6 < mn) mn = r6;
    if (r6 > mx) mx = r6;
    double den = mx - mn;
    if (den <= ZERO_EPS) return 0.0;
    double invDen = 1.0 / den;
    double v0 = (r0 - mn) * invDen;
    double v1 = (r1 - mn) * invDen;
    double v2 = (r2 - mn) * invDen;
    double v3 = (r3 - mn) * invDen;
    double v4 = (r4 - mn) * invDen;
    double v5 = (r5 - mn) * invDen;
    double v6 = (r6 - mn) * invDen;
    double ss = v0*v0 + v1*v1 + v2*v2 + v3*v3 + v4*v4 + v5*v5 + v6*v6;
    if (ss <= 0.0) return 0.0;
    double dot = q->st[0]*v0 + q->st[1]*v1 + q->st[2]*v2 + q->st[3]*v3 +
                 q->st[4]*v4 + q->st[5]*v5 + q->st[6]*v6;
    return dot * q->stInvNorm / dsqrt2(ss);
}

static void process_doc_word(WorkCtx *ctx, const char *span, int len, unsigned long long h,
                             double *semDot, double *semSq) {
    localwords_add(&ctx->words, len, h, semSq);
    QSlot *q = qset_get_lower_span(ctx->qset, span, len, h);
    if (q) {
        *semDot += (double)q->cnt;
        if (!q->appeared) {
            q->appeared = 1;
            ctx->qSemSq += (double)q->cnt * q->cnt;
        }
    }
}

static Sentence make_scanned_sentence(const char *outText, int outLen, int idx, const Query *q,
                                      int needStruct, int needSem,
                                      int wordN, double sumLen, double sumSqLen, double sumVowelRatio,
                                      int validChars, int punctN, int upperN, int digitN,
                                      double semDot, double semSq) {
    double r0 = 0.0, r1 = 0.0, r2 = 0.0, r3 = 0.0, r4 = 0.0, r5 = 0.0, r6 = 0.0;
    if (needStruct) {
        if (wordN > 0) {
            double invWordN = wordN < INV_LEN_CAP ? T_INV_LEN[wordN] : 1.0 / (double)wordN;
            double avg = sumLen * invWordN;
            r0 = avg;
            r1 = sumSqLen * invWordN - avg * avg;
            if (r1 < 0.0 && r1 > -1e-9) r1 = 0.0;
            r2 = (double)wordN;
            r3 = sumVowelRatio * invWordN;
        }
        if (validChars > 0) {
            double invChars = validChars < INV_LEN_CAP ? T_INV_LEN[validChars] : 1.0 / (double)validChars;
            r4 = (double)punctN * invChars;
            r5 = (double)upperN * invChars;
            r6 = (double)digitN * invChars;
        }
    }

    Sentence sent;
    sent.text = outText;
    sent.len = outLen;
    sent.idx = idx;
    sent.semBase = (needSem && semSq > 0.0 && semDot != 0.0) ? semDot / dsqrt2(semSq) : 0.0;
    sent.stScore = needStruct ? structure_score_vals(r0, r1, r2, r3, r4, r5, r6, q) : 0.0;
    return sent;
}

/* parse query string */
static void build_query(Query *q, const char *s, const WordSet *stop) {
    qset_init(&q->qset);
    for (int i = 0; i < 7; ++i) q->raw[i] = q->st[i] = 0.0;
    q->stInvNorm = 0.0;
    q->semInvNorm = 0.0;

    int n = (int)strlen(s);
    int validChars = 0, punctN = 0, upperN = 0, digitN = 0;
    int wordN = 0;
    double sumLen = 0.0, sumSqLen = 0.0, sumVowelRatio = 0.0;

    int tokCap = g_tok_cap, tokLen = 0, tokVowels = 0;
    unsigned long long h = HASH_INIT;
    char *tok = get_token_buf(64);
    tokCap = g_tok_cap;

    for (int i = 0; i <= n; ++i) {
        unsigned char c = 0, fl = F_SEP;
        if (i < n) {
            c = (unsigned char)s[i];
            fl = T_FLAG[c];
            validChars += (fl & F_SPACE) == 0;
            punctN += (fl & F_PUNCT) != 0;
            upperN += (fl & F_UPPER) != 0;
            digitN += (fl & F_DIGIT) != 0;
        }

        if (fl & F_SEP) {
            if (tokLen > 0) {
                if (h == 0) h = 1;
                if (!wordset_contains_len(stop, tok, tokLen, h)) {
                    ++wordN;
                    sumLen += tokLen;
                    sumSqLen += (double)tokLen * tokLen;
                    sumVowelRatio += tokLen < INV_LEN_CAP ? tokVowels * T_INV_LEN[tokLen] : (double)tokVowels / tokLen;
                    qset_add(&q->qset, tok, tokLen, h);
                }
                tokLen = 0;
                tokVowels = 0;
                h = HASH_INIT;
            }
        } else {
            if (tokLen + 1 >= tokCap) {
                tokCap <<= 1;
                tok = get_token_buf(tokCap);
            }
            unsigned char lc = T_LOWER[c];
            tok[tokLen++] = (char)lc;
            tokVowels += (T_FLAG[lc] & F_VOWEL) != 0;
            h = HASH_STEP(h, lc);
        }
    }
    if (wordN > 0) {
        double invWordN = wordN < INV_LEN_CAP ? T_INV_LEN[wordN] : 1.0 / (double)wordN;
        double avg = sumLen * invWordN;
        q->raw[0] = avg;
        q->raw[1] = sumSqLen * invWordN - avg * avg;
        if (q->raw[1] < 0.0 && q->raw[1] > -1e-9) q->raw[1] = 0.0;
        q->raw[2] = (double)wordN;
        q->raw[3] = sumVowelRatio * invWordN;
    }
    if (validChars > 0) {
        double invChars = validChars < INV_LEN_CAP ? T_INV_LEN[validChars] : 1.0 / (double)validChars;
        q->raw[4] = (double)punctN * invChars;
        q->raw[5] = (double)upperN * invChars;
        q->raw[6] = (double)digitN * invChars;
    }
    normalize7(q->raw, q->st, &q->stInvNorm);
}

static void build_document(const char *doc, int len, const WordSet *stop, WorkCtx *ctx, const Query *q,
                           SentArray *sa, int needSem, int needStruct) {
    int idx = 0;

    int firstNonSpace = -1;
    int lastNonSpaceEnd = -1;
    int validChars = 0, punctN = 0, upperN = 0, digitN = 0;
    int wordN = 0;
    double sumLen = 0.0, sumSqLen = 0.0, sumVowelRatio = 0.0;
    double semDot = 0.0, semSq = 0.0;

    int tokStart = 0, tokLen = 0, tokVowels = 0;
    unsigned long long h = HASH_INIT;

    if (needSem) localwords_reset(&ctx->words);

    for (int i = 0; i <= len; ++i) {
        unsigned char c = 0;
        unsigned char fl = F_SEP | F_SENT;
        if (i < len) {
            c = (unsigned char)doc[i];
            fl = T_FLAG[c];
            if (!(fl & F_SPACE)) {
                if (firstNonSpace < 0) firstNonSpace = i;
                lastNonSpaceEnd = i + 1;
            }
            if (needStruct) {
                validChars += (fl & F_SPACE) == 0;
                punctN += (fl & F_PUNCT) != 0;
                upperN += (fl & F_UPPER) != 0;
                digitN += (fl & F_DIGIT) != 0;
            }
        }

        if (fl & F_SEP) {
            if (tokLen > 0) {
                if (h == 0) h = 1;
                const char *tok = doc + tokStart;
                if (!wordset_contains_lower_span(stop, tok, tokLen, h)) {
                    if (needStruct) {
                        ++wordN;
                        sumLen += tokLen;
                        sumSqLen += (double)tokLen * tokLen;
                        sumVowelRatio += tokLen < INV_LEN_CAP ? tokVowels * T_INV_LEN[tokLen] : (double)tokVowels / (double)tokLen;
                    }
                    if (needSem) process_doc_word(ctx, tok, tokLen, h, &semDot, &semSq);
                }
                tokLen = 0;
                tokVowels = 0;
                h = HASH_INIT;
            }
        } else {
            unsigned char lc = T_LOWER[c];
            if (tokLen == 0) tokStart = i;
            ++tokLen;
            if (needStruct) tokVowels += (T_FLAG[lc] & F_VOWEL) != 0;
            h = HASH_STEP(h, lc);
        }

        if (fl & F_SENT) {
            if (firstNonSpace >= 0) {
                Sentence s = make_scanned_sentence(doc + firstNonSpace, lastNonSpaceEnd - firstNonSpace,
                                                   idx++, q, needStruct, needSem,
                                                   wordN, sumLen, sumSqLen, sumVowelRatio,
                                                   validChars, punctN, upperN, digitN,
                                                   semDot, semSq);
                sentarray_push(sa, s);
            }

            firstNonSpace = -1;
            lastNonSpaceEnd = -1;
            validChars = 0;
            punctN = 0;
            upperN = 0;
            digitN = 0;
            wordN = 0;
            sumLen = 0.0;
            sumSqLen = 0.0;
            sumVowelRatio = 0.0;
            semDot = 0.0;
            semSq = 0.0;
            tokLen = 0;
            tokVowels = 0;
            h = HASH_INIT;
            if (needSem) localwords_reset(&ctx->words);
        }
    }
}

/* ---------- top K ---------- */

static int better_sentence(Sentence a, Sentence b) {
    if (a.score > b.score) return 1;
    if (a.score < b.score) return 0;
    return a.idx < b.idx;
}

static void swap_sentence(Sentence *a, Sentence *b) {
    Sentence t = *a;
    *a = *b;
    *b = t;
}

static int cmp_sentence(const void *pa, const void *pb) {
    Sentence a = *(const Sentence *)pa;
    Sentence b = *(const Sentence *)pb;
    if (better_sentence(a, b)) return -1;
    if (better_sentence(b, a)) return 1;
    return 0;
}

/* ---------- quickselect Top-K ---------- */

static Sentence median3_sentence(Sentence a, Sentence b, Sentence c) {
    if (better_sentence(b, a)) { Sentence t = a; a = b; b = t; }
    if (better_sentence(c, b)) {
        Sentence t = b; b = c; c = t;
        if (better_sentence(b, a)) { t = a; a = b; b = t; }
    }
    return b;
}

/*
 * Put the best k elements in a[0..k-1]. The selected prefix is not sorted.
 * The ranking uses better(), so equal scores are still ordered by lower idx.
 */
static void quickselect_topk(Sentence *a, int n, int k) {
    if (k <= 0 || k >= n) return;

    int target = k - 1;
    int l = 0, r = n - 1;

    while (l <= r) {
        int mid = l + ((r - l) >> 1);
        Sentence pivot = median3_sentence(a[l], a[mid], a[r]);

        int lt = l;
        int i = l;
        int gt = r;

        while (i <= gt) {
            if (better_sentence(a[i], pivot)) {
                swap_sentence(&a[lt], &a[i]);
                ++lt;
                ++i;
            } else if (better_sentence(pivot, a[i])) {
                swap_sentence(&a[i], &a[gt]);
                --gt;
            } else {
                ++i;
            }
        }

        if (target < lt) {
            r = lt - 1;
        } else if (target > gt) {
            l = gt + 1;
        } else {
            return;
        }
    }
}

int main(void) {
    init_char_table();

    char *queryLine = read_line_stdin();
    char *alphaLine = read_nonblank_stdin();
    char *kLine = read_nonblank_stdin();
    if (!queryLine || !alphaLine || !kLine) return 0;

    double alpha = atof(alphaLine);
    int K = atoi(kLine);
    free(alphaLine);
    free(kLine);

    WordSet stop;
    load_stopwords(&stop, "stopList.txt");

    Query query;
    build_query(&query, queryLine, &stop);

    int needSem = alpha > ZERO_EPS;
    int needStruct = alpha < 1.0 - ZERO_EPS;

    WorkCtx ctx = {0};
    ctx.qset = &query.qset;
    ctx.qSemSq = 0.0;
    if (needSem) localwords_init(&ctx.words);

    long docLenLong;
    char *doc = read_all_file("document.txt", &docLenLong);

    SentArray sa;
    sentarray_init(&sa);
    build_document(doc, (int)docLenLong, &stop, &ctx, &query, &sa, needSem, needStruct);

    query.semInvNorm = ctx.qSemSq > 0.0 ? 1.0 / dsqrt2(ctx.qSemSq) : 0.0;

    if (K > sa.n) K = sa.n;
    if (K < 0) K = 0;

    if (K == 0) return 0;

    double semWeight = alpha * query.semInvNorm;
    double stWeight = 1.0 - alpha;
    for (int i = 0; i < sa.n; ++i) {
        sa.a[i].score = semWeight * sa.a[i].semBase + stWeight * sa.a[i].stScore;
    }

    if (K == 1) {
        Sentence best = sa.a[0];
        for (int i = 1; i < sa.n; ++i) {
            if (better_sentence(sa.a[i], best)) best = sa.a[i];
        }
        fwrite(best.text, 1, (size_t)best.len, stdout);
        putchar('\n');
        return 0;
    }

    if (K * 2 >= sa.n) {
        qsort(sa.a, sa.n, sizeof(Sentence), cmp_sentence);
        for (int i = 0; i < K; ++i) {
            fwrite(sa.a[i].text, 1, (size_t)sa.a[i].len, stdout);
            putchar('\n');
        }
        return 0;
    }

    if (K <= 16) {
        Sentence top[16];
        int tn = 0;
        for (int i = 0; i < sa.n; ++i) {
            Sentence r = sa.a[i];

            if (tn < K || better_sentence(r, top[tn - 1])) {
                int p = tn < K ? tn++ : K - 1;
                while (p > 0 && better_sentence(r, top[p - 1])) {
                    top[p] = top[p - 1];
                    --p;
                }
                top[p] = r;
            }
        }
        for (int i = 0; i < tn; ++i) {
            fwrite(top[i].text, 1, (size_t)top[i].len, stdout);
            putchar('\n');
        }
        return 0;
    }

    quickselect_topk(sa.a, sa.n, K);
    qsort(sa.a, K, sizeof(Sentence), cmp_sentence);
    for (int i = 0; i < K; ++i) {
        fwrite(sa.a[i].text, 1, (size_t)sa.a[i].len, stdout);
        putchar('\n');
    }

    return 0;
}

