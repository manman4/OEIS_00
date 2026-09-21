/*
 * Number of permutations of {1,2,...,n} for which sums of adjacent numbers
 * are all distinct.
 *   （p_1 + p_2, p_2 + p_3, ..., p_{n-1} + p_n の n-1 個がすべて異なる順列の個数）
 *
 * コンパイルと実行
 *   gcc -O3 -march=native -o 091217_01 091217_01.c
 *   ./091217_01 [N]        # n = 1..N の "n a(n)" を標準出力へ（既定 N = 15, 最大 20）
 *
 *   目安（1 コア）: n = 14 で約 20 秒、n = 15 で約 4 分。n が 1 増えるごとに約 10 倍。
 *
 * 方針
 *   先頭から 1 つずつ数を置く深さ優先探索。使った数と、すでに現れた隣接和を
 *   ビットマスクで持ち、新しい和が既出なら枝を切る。
 *
 * 高速化
 *   - 補数対称性：p_i -> n+1-p_i で和は s -> 2n+2-s に写り、条件が保たれる。
 *     先頭 f と n+1-f の個数は等しいので、f <= (n+1)/2 だけ数えて 2 倍する
 *     （f = (n+1)/2 のときは自身と対応するので 1 倍）。
 *   - 残りが R 個以下になった局面をキャッシュする。その先の数え上げに影響するのは
 *     「残りの数の集合」「最後の数」と、既出の和のうち今後現れうるもの
 *     （最後の数 + 残りの数、残りの数どうしの和）だけなので、それをキーにする。
 *     キャッシュは上書き型（衝突したら捨てる）で、メモリ使用量は一定。
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned long long u64;

#define MAXN 20
#define R 7            /* 残りがこの個数以下の局面をキャッシュする */
#define CBITS 22       /* キャッシュのエントリ数 2^CBITS（16 バイト/エントリ） */

static int n;
static u64 full;       /* ビット v (1 <= v <= n) が立ったマスク */
static u64 *ckey, *cval;

/*
 * last  : 最後に置いた数
 * used  : 使った数の集合（ビット v）
 * sums  : 既出の隣接和の集合（ビット s）
 * left  : 残りの個数
 * 戻り値: 残りの並べ方のうち条件を満たすものの個数
 */
static u64 count(int last, u64 used, u64 sums, int left)
{
    u64 avail = full & ~used;

    if (left == 1) {
        int v = __builtin_ctzll(avail);
        return !((sums >> (last + v)) & 1);
    }

    u64 key = 0;
    size_t slot = 0;
    if (left <= R) {
        /* 今後現れうる和だけを残す */
        u64 rel = 0, a = avail;
        while (a) {
            int v = __builtin_ctzll(a);
            a &= a - 1;
            rel |= 1ULL << (last + v);
            u64 b = a;
            while (b) {
                int w = __builtin_ctzll(b);
                b &= b - 1;
                rel |= 1ULL << (v + w);
            }
        }
        /* キー: [既出の関連和 (ビット 3..2n-1 を 0 始まりに)] [残りの集合] [最後の数] */
        key = ((sums & rel) >> 3) << (n + 5);
        key |= (avail >> 1) << 5;
        key |= (u64)last;
        key += 1; /* 0 を空きの印にする */
        slot = (size_t)((key * 0x9E3779B97F4A7C15ULL) >> (64 - CBITS));
        if (ckey[slot] == key)
            return cval[slot];
    }

    u64 c = 0, a = avail;
    while (a) {
        int v = __builtin_ctzll(a);
        a &= a - 1;
        int s = last + v;
        if ((sums >> s) & 1)
            continue;
        c += count(v, used | (1ULL << v), sums | (1ULL << s), left - 1);
    }

    if (left <= R) {
        ckey[slot] = key;
        cval[slot] = c;
    }
    return c;
}

static u64 a091217(int m)
{
    n = m;
    full = 0;
    for (int v = 1; v <= n; v++)
        full |= 1ULL << v;
    for (size_t i = 0; i < ((size_t)1 << CBITS); i++)
        ckey[i] = 0; /* n が変わるとキーの意味が変わるので消す */

    if (n == 1)
        return 1;

    u64 total = 0;
    for (int f = 1; 2 * f <= n + 1; f++) {
        u64 c = count(f, 1ULL << f, 0, n - 1);
        total += (2 * f < n + 1) ? 2 * c : c;
    }
    return total;
}

int main(int argc, char **argv)
{
    int nmax = 15;
    if (argc > 1)
        nmax = atoi(argv[1]);
    if (nmax < 1 || nmax > MAXN) {
        fprintf(stderr, "N must be in 1..%d\n", MAXN);
        return 1;
    }

    ckey = calloc((size_t)1 << CBITS, sizeof(u64));
    cval = calloc((size_t)1 << CBITS, sizeof(u64));
    if (!ckey || !cval) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    clock_t t0 = clock();
    for (int m = 1; m <= nmax; m++) {
        u64 v = a091217(m);
        printf("%d %llu\n", m, v);
        fflush(stdout);
        fprintf(stderr, "n=%d done (%.2f s)\n", m, (double)(clock() - t0) / CLOCKS_PER_SEC);
    }
    return 0;
}
