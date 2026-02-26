#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#define MAXN 10000

// dp[r][l] = remaining sum r, last element l のときの有効完成パターン数
// flat配列: dp[r*(MAXN+1)+l]
static mpz_t *dp;
static mpz_t suf_tmp[MAXN + 2];

#define DP(r,l) dp[(long long)(r)*(MAXN+1)+(l)]

static void precompute(int max_n) {
    // suf_tmp 初期化
    for (int j = 0; j <= max_n + 1; j++)
        mpz_init(suf_tmp[j]);

    // 基底: 残余和=0 なら完成 -> 1通り
    for (int l = 1; l <= max_n; l++)
        mpz_set_ui(DP(0, l), 1);

    for (int r = 1; r <= max_n; r++) {
        // suf_tmp[j] = sum_{k=j}^{r} dp[r-k][k]
        mpz_set_ui(suf_tmp[r + 1], 0);
        for (int j = r; j >= 1; j--)
            mpz_add(suf_tmp[j], suf_tmp[j + 1], DP(r - j, j));

        // dp[r][l] = suf_tmp[min_next(l)]
        // min_next(l): l奇数なら l+1(偶数), l偶数なら l
        for (int l = 1; l <= r; l++) {
            int mn = l + (l & 1);
            if (mn <= r)
                mpz_set(DP(r, l), suf_tmp[mn]);
            // else dp[r][l] remains 0
        }
    }
}

static void A006950(mpz_t result, int n) {
    if (n <= 0) { mpz_set_ui(result, 1); return; }
    mpz_set_ui(result, 0);
    for (int a1 = 1; a1 <= n; a1++)
        mpz_add(result, result, DP(n - a1, a1));
}

int main() {
    long long size = (long long)(MAXN + 1) * (MAXN + 1);
    dp = malloc(size * sizeof(mpz_t));
    if (!dp) { fprintf(stderr, "malloc failed\n"); return 1; }
    for (long long i = 0; i < size; i++)
        mpz_init(dp[i]);

    precompute(MAXN);

    FILE *fp = fopen("b006950_1.txt", "w");
    mpz_t val;
    mpz_init(val);
    for (int i = 0; i <= MAXN; i++) {
        A006950(val, i);
        gmp_fprintf(fp, "%d %Zd\n", i, val);
    }
    mpz_clear(val);
    fclose(fp);

    for (long long i = 0; i < size; i++)
        mpz_clear(dp[i]);
    free(dp);

    return 0;
}
