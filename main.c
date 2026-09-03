#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define COIN_KINDS   5U
#define TOY_COUNT    4U
#define NAME_LEN    16U

static const uint16_t COIN_VALUE[COIN_KINDS] = { 25U, 50U, 100U, 200U, 500U };
static uint16_t coinCount[COIN_KINDS];

typedef struct {
    char     name[NAME_LEN];
    uint32_t price;
} Toy_t;

static Toy_t shop[TOY_COUNT];

static int readInt(int *out)
{
    char buf[64];
    if (fgets(buf, (int)sizeof(buf), stdin) == NULL) {
        return 0;
    }
    return sscanf(buf, "%d", out) == 1;
}

static void seedBank(void)
{
    coinCount[0] = 4U;
    coinCount[1] = 6U;
    coinCount[2] = 5U;
    coinCount[3] = 2U;
    coinCount[4] = 1U;

    const char *names[TOY_COUNT] = { "Sticker Pack", "Toy Car", "Puzzle Box", "Robot Drone" };
    const uint32_t prices[TOY_COUNT] = { 150U, 500U, 1200U, 2500U };

    for (uint8_t i = 0U; i < TOY_COUNT; i++) {
        strncpy(shop[i].name, names[i], NAME_LEN - 1U);
        shop[i].name[NAME_LEN - 1U] = '\0';
        shop[i].price = prices[i];
    }
}

static void drawBar(uint16_t value, uint16_t full, uint8_t width)
{
    uint8_t filled = 0U;
    if (full > 0U) {
        filled = (value >= full) ? width : (uint8_t)(((uint32_t)value * width) / full);
    }
    for (uint8_t i = 0U; i < width; i++) {
        putchar(i < filled ? '#' : '-');
    }
}

static void addCoins(void)
{
    int kind = 0;
    int count = 0;

    printf("Coin (0=25p, 1=50p, 2=100p, 3=200p, 4=500p): ");
    if (!readInt(&kind) || kind < 0 || kind >= (int)COIN_KINDS) {
        printf("Invalid coin!\n");
        return;
    }

    printf("Count to add: ");
    if (!readInt(&count) || count <= 0) {
        printf("Invalid count!\n");
        return;
    }

    coinCount[kind] += (uint16_t)count;
    printf("Added.\n");
}

static void takeCoins(void)
{
    int kind = 0;
    int count = 0;

    printf("Coin to take (0..4): ");
    if (!readInt(&kind) || kind < 0 || kind >= (int)COIN_KINDS) {
        printf("Invalid coin!\n");
        return;
    }

    printf("Count to take: ");
    if (!readInt(&count) || count <= 0) {
        printf("Invalid count!\n");
        return;
    }

    if ((uint16_t)count > coinCount[kind]) {
        printf("Refused! Not enough coins.\n");
        return;
    }

    coinCount[kind] -= (uint16_t)count;
    printf("Coins removed.\n");
}

static uint32_t bankTotal(void)
{
    uint32_t total = 0U;
    for (uint8_t i = 0U; i < COIN_KINDS; i++) {
        total += (uint32_t)coinCount[i] * COIN_VALUE[i];
    }
    return total;
}

static uint32_t sumCoins(const uint16_t *counts, uint8_t n)
{
    if (n == 0U || counts == NULL) {
        return 0U;
    }
    return (uint32_t)counts[n - 1U] + sumCoins(counts, n - 1U);
}

static uint8_t biggestPile(void)
{
    uint8_t best = 0U;
    for (uint8_t i = 1U; i < COIN_KINDS; i++) {
        if (coinCount[i] > coinCount[best]) {
            best = i;
        }
    }
    return best;
}

static void showBank(void)
{
    uint16_t max = 1U;
    for (uint8_t i = 0U; i < COIN_KINDS; i++) {
        if (coinCount[i] > max) {
            max = coinCount[i];
        }
    }

    printf("\n--- Piggy Bank ---\n");
    for (uint8_t i = 0U; i < COIN_KINDS; i++) {
        printf("%3u piastres: %3u [", COIN_VALUE[i], coinCount[i]);
        drawBar(coinCount[i], max, 12U);
        printf("]\n");
    }
    printf("Total: %u piastres\n", bankTotal());
}

static void buyToy(void)
{
    printf("\n--- Toy Shop ---\n");
    for (uint8_t i = 0U; i < TOY_COUNT; i++) {
        printf("%u) %-12s : %u piastres\n", i, shop[i].name, shop[i].price);
    }

    int choice = 0;
    printf("Choose toy (0..3): ");
    if (!readInt(&choice) || choice < 0 || choice >= (int)TOY_COUNT) {
        printf("Invalid choice!\n");
        return;
    }

    uint32_t total = bankTotal();
    if (total >= shop[choice].price) {
        printf("You can buy %s! Remaining: %u piastres.\n",
               shop[choice].name, total - shop[choice].price);
    } else {
        printf("Cannot afford. You need %u more piastres.\n",
               shop[choice].price - total);
    }
}

static void bankReport(void)
{
    uint32_t total = bankTotal();
    uint8_t pile = biggestPile();
    uint8_t affordable = 0U;

    for (uint8_t i = 0U; i < TOY_COUNT; i++) {
        if (total >= shop[i].price) {
            affordable++;
        }
    }

    printf("\n=== Bank Report ===\n");
    printf("Total Money : %u piastres\n", total);
    printf("Total Coins : %u\n", sumCoins(coinCount, COIN_KINDS));
    printf("Tallest Pile: %u piastres (%u coins)\n", COIN_VALUE[pile], coinCount[pile]);
    printf("Can Afford  : %u of %u toys\n", affordable, TOY_COUNT);
}

int main(void)
{
    seedBank();
    int opt = -1;

    do {
        printf("\n1.Show 2.Add 3.Take 4.Buy 5.Report 6.Reset 0.Exit > ");
        if (!readInt(&opt)) {
            printf("Invalid input!\n");
            continue;
        }

        switch (opt) {
            case 1: showBank();   break;
            case 2: addCoins();   break;
            case 3: takeCoins();  break;
            case 4: buyToy();     break;
            case 5: bankReport(); break;
            case 6: seedBank(); printf("Bank reset.\n"); break;
            case 0: printf("Exiting...\n"); break;
            default: printf("Unknown option!\n"); break;
        }
    } while (opt != 0);

    return 0;
}