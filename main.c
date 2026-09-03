#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TICKS_GREEN    5U
#define TICKS_YELLOW   2U
#define TICKS_RED      4U
#define QUEUE_BUSY     6U
#define LOG_LEN       20U

typedef enum { LIGHT_GREEN = 0, LIGHT_YELLOW, LIGHT_RED } LightState_t;

/* Status bits */
#define BIT_NIGHT      0U
#define BIT_BUSY       1U
#define BIT_BLINK_ON   2U

#define SET_BIT(reg, n)    ((reg) |=  (uint8_t)(1U << (n)))
#define CLR_BIT(reg, n)    ((reg) &= (uint8_t)~(1U << (n)))
#define TOGGLE_BIT(reg, n) ((reg) ^=  (uint8_t)(1U << (n)))
#define READ_BIT(reg, n)   ((uint8_t)(((reg) >> (n)) & 1U))

static LightState_t light;
static uint8_t      status;
static uint8_t      ticksLeft;
static uint8_t      carsWaiting;
static uint32_t     carsPassed;
static char         logLine[LOG_LEN + 1U];

/* Function Prototypes */
static void         resetCrossing(void);
static uint8_t      ticksFor(LightState_t s);
static LightState_t nextState(LightState_t s);
static void         drawLight(void);
static void         tick(void);
static void         addCars(void);
static void         toggleNight(void);
static void         pushLog(char c);
static void         showLog(void);
static void         crossingReport(void);

static int readInt(int *out)
{
    char buf[64];
    if (fgets(buf, (int)sizeof(buf), stdin) == NULL) {
        return 0;
    }
    return sscanf(buf, "%d", out) == 1;
}

static uint8_t ticksFor(LightState_t s)
{
    if (s == LIGHT_GREEN) {
        return READ_BIT(status, BIT_BUSY) ? (TICKS_GREEN + 2U) : TICKS_GREEN;
    }
    if (s == LIGHT_YELLOW) {
        return TICKS_YELLOW;
    }
    return TICKS_RED;
}

static LightState_t nextState(LightState_t s)
{
    if (s == LIGHT_GREEN) {
        return LIGHT_YELLOW;
    }
    if (s == LIGHT_YELLOW) {
        return LIGHT_RED;
    }
    return LIGHT_GREEN;
}

static void pushLog(char c)
{
    for (uint8_t i = 0U; i < LOG_LEN - 1U; i++) {
        logLine[i] = logLine[i + 1U];
    }
    logLine[LOG_LEN - 1U] = c;
    logLine[LOG_LEN] = '\0';
}

static void resetCrossing(void)
{
    light = LIGHT_RED;
    status = 0U;
    ticksLeft = ticksFor(LIGHT_RED);
    carsWaiting = 0U;
    carsPassed = 0U;
    memset(logLine, '-', LOG_LEN);
    logLine[LOG_LEN] = '\0';
}

static void drawLight(void)
{
    printf("\n+---+\n");
    if (READ_BIT(status, BIT_NIGHT)) {
        printf("|   |\n");
        printf("| %c |\n", READ_BIT(status, BIT_BLINK_ON) ? '*' : 'o');
        printf("|   |\n");
        printf("+---+\n[NIGHT BLINK]\n");
    } else {
        printf("| %c | Red\n",    (light == LIGHT_RED)    ? 'O' : '.');
        printf("| %c | Yellow\n", (light == LIGHT_YELLOW) ? 'O' : '.');
        printf("| %c | Green\n",  (light == LIGHT_GREEN)  ? 'O' : '.');
        printf("+---+\n");
        printf("Ticks left: %u | Cars waiting: %u\n", ticksLeft, carsWaiting);
    }
}

static void tick(void)
{
    if (READ_BIT(status, BIT_NIGHT)) {
        TOGGLE_BIT(status, BIT_BLINK_ON);
        pushLog(READ_BIT(status, BIT_BLINK_ON) ? 'Y' : 'y');
        return;
    }

    if (light == LIGHT_GREEN && carsWaiting > 0U) {
        uint8_t passing = (carsWaiting >= 2U) ? 2U : carsWaiting;
        carsWaiting -= passing;
        carsPassed += passing;
        if (carsWaiting <= QUEUE_BUSY) {
            CLR_BIT(status, BIT_BUSY);
        }
    }

    char c = (light == LIGHT_GREEN) ? 'G' : (light == LIGHT_YELLOW ? 'Y' : 'R');
    pushLog(c);

    ticksLeft--;
    if (ticksLeft == 0U) {
        light = nextState(light);
        ticksLeft = ticksFor(light);
    }
}

static void addCars(void)
{
    int count = 0;
    printf("How many cars arrived? ");
    if (!readInt(&count) || count < 0 || count > 50) {
        printf("Invalid count (0..50)!\n");
        return;
    }

    carsWaiting += (uint8_t)count;
    if (carsWaiting > QUEUE_BUSY) {
        SET_BIT(status, BIT_BUSY);
    }
    printf("%d cars added. Total waiting: %u\n", count, carsWaiting);
}

static void toggleNight(void)
{
    TOGGLE_BIT(status, BIT_NIGHT);
    if (READ_BIT(status, BIT_NIGHT)) {
        SET_BIT(status, BIT_BLINK_ON);
        printf("Switched to NIGHT mode.\n");
    } else {
        CLR_BIT(status, BIT_BLINK_ON);
        light = LIGHT_RED;
        ticksLeft = ticksFor(LIGHT_RED);
        printf("Returned to DAY mode (Red light reset).\n");
    }
}

static void showLog(void)
{
    printf("\nRecent Log (Oldest -> Newest):\n[%s]\n", logLine);
}

static void crossingReport(void)
{
    printf("\n=== Crossing Report ===\n");
    printf("Cars Passed  : %u\n", carsPassed);
    printf("Cars Waiting : %u\n", carsWaiting);
    printf("Mode         : %s\n", READ_BIT(status, BIT_NIGHT) ? "Night" : "Day");
    printf("Traffic      : %s\n", READ_BIT(status, BIT_BUSY)  ? "Busy (>6 cars)" : "Normal");
    printf("Status Byte  : 0b");
    for (int8_t b = 7; b >= 0; b--) {
        putchar(READ_BIT(status, b) ? '1' : '0');
    }
    printf(" (0x%02X)\n", status);
}

int main(void)
{
    resetCrossing();
    int opt = -1;

    do {
        printf("\n1.Draw 2.Tick(1s) 3.Tick(10s) 4.AddCars 5.Night 6.Log 7.Report 0.Exit > ");
        if (!readInt(&opt)) {
            printf("Invalid input!\n");
            continue;
        }

        switch (opt) {
            case 1: drawLight(); break;
            case 2: tick(); drawLight(); break;
            case 3:
                for (uint8_t i = 0U; i < 10U; i++) { tick(); }
                drawLight();
                break;
            case 4: addCars(); break;
            case 5: toggleNight(); break;
            case 6: showLog(); break;
            case 7: crossingReport(); break;
            case 0: printf("Exiting...\n"); break;
            default: printf("Unknown option!\n"); break;
        }
    } while (opt != 0);

    return 0;
}