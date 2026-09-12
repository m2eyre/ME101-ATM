#pragma region VEXcode Generated Robot Configuration
// Make sure all required headers are included.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>


#include "vex.h"

using namespace vex;

// Brain should be defined by default
brain Brain;


// START IQ MACROS
#define waitUntil(condition)                                                   \
  do {                                                                         \
    wait(5, msec);                                                             \
  } while (!(condition))

#define repeat(iterations)                                                     \
  for (int iterator = 0; iterator < iterations; iterator++)
// END IQ MACROS


// Robot configuration code.
inertial BrainInertial = inertial();
motor Motor1 = motor(PORT1, false);
optical Optical3 = optical(PORT3);
motor Motor6 = motor(PORT6, false);
motor Motor2 = motor(PORT2, false);
bumper Bumper4 = bumper(PORT4);


// generating and setting random seed
void initializeRandomSeed(){
  wait(100,msec);
  double xAxis = BrainInertial.acceleration(xaxis) * 1000;
  double yAxis = BrainInertial.acceleration(yaxis) * 1000;
  double zAxis = BrainInertial.acceleration(zaxis) * 1000;
  // Combine these values into a single integer
  int seed = int(
    xAxis + yAxis + zAxis
  );
  // Set the seed
  srand(seed); 
}

// Converts a color to a string
const char* convertColorToString(color col) {
  if (col == colorType::red) return "red";
  else if (col == colorType::green) return "green";
  else if (col == colorType::blue) return "blue";
  else if (col == colorType::white) return "white";
  else if (col == colorType::yellow) return "yellow";
  else if (col == colorType::orange) return "orange";
  else if (col == colorType::purple) return "purple";
  else if (col == colorType::cyan) return "cyan";
  else if (col == colorType::black) return "black";
  else if (col == colorType::transparent) return "transparent";
  else if (col == colorType::red_violet) return "red_violet";
  else if (col == colorType::violet) return "violet";
  else if (col == colorType::blue_violet) return "blue_violet";
  else if (col == colorType::blue_green) return "blue_green";
  else if (col == colorType::yellow_green) return "yellow_green";
  else if (col == colorType::yellow_orange) return "yellow_orange";
  else if (col == colorType::red_orange) return "red_orange";
  else if (col == colorType::none) return "none";
  else return "unknown";
}


// Convert colorType to string
const char* convertColorToString(colorType col) {
  if (col == colorType::red) return "red";
  else if (col == colorType::green) return "green";
  else if (col == colorType::blue) return "blue";
  else if (col == colorType::white) return "white";
  else if (col == colorType::yellow) return "yellow";
  else if (col == colorType::orange) return "orange";
  else if (col == colorType::purple) return "purple";
  else if (col == colorType::cyan) return "cyan";
  else if (col == colorType::black) return "black";
  else if (col == colorType::transparent) return "transparent";
  else if (col == colorType::red_violet) return "red_violet";
  else if (col == colorType::violet) return "violet";
  else if (col == colorType::blue_violet) return "blue_violet";
  else if (col == colorType::blue_green) return "blue_green";
  else if (col == colorType::yellow_green) return "yellow_green";
  else if (col == colorType::yellow_orange) return "yellow_orange";
  else if (col == colorType::red_orange) return "red_orange";
  else if (col == colorType::none) return "none";
  else return "unknown";
}


void vexcodeInit() {

  // Initializing random seed.
  initializeRandomSeed(); 
}

#pragma endregion VEXcode Generated Robot Configuration

//----------------------------------------------------------------------------
// Module:       main.cpp
// Description:  VEX IQ "ATM" coin manager
//
// 1) Boot + calibrate inertial
// 2) UI state machine (UserSelect -> Menu -> Deposit/Withdraw)
// 3) Deposit: optical sensor detects coin colors and updates balance + stock
// 4) Withdraw: user selects amount + step; system computes coin plan and dispenses
// 5) Safety: tilt-based tamper detection triggers LOCKDOWN until "Check" held
//----------------------------------------------------------------------------

// Include the IQ Library (VEX IQ helpers)
#include "iq_cpp.h"
#include <string>

using namespace vex;

//------------------------------------------------------------------------------
// Forward declarations 
//------------------------------------------------------------------------------

// UI drawing helpers
void drawHazardIcon(int x, int y, int size);
void drawLockdownFlashing(bool visible);

//------------------------------------------------------------------------------
// Constants / configuration
//------------------------------------------------------------------------------

// Coin values (in cents)
const int RED_CENTS   = 100; // $1.00
const int GREEN_CENTS = 25;  // $0.25
const int BLUE_CENTS  = 5;   // $0.05

// Auto-return to User Select after inactivity
//const int INACTIVITY_TIMEOUT_MS = 20000;  // 20 seconds       IMPLIMENT LATER IF WE WANT

// Tamper detection thresholds
const double TILT_DEG      = 25.0;  // Pitch/roll threshold to consider "tilted"
const int    TAMPER_HOLD_MS = 300;  // Must stay tilted this long to trigger lockdown
const int    UNLOCK_HOLD_MS = 2000; // Hold Check this long to unlock

// Deposit anti-double-count settings
int MIN_TIME_BETWEEN_COUNTS_MS = 350; // Extra debounce in case object flickers

// Number of ATM user accounts
const int NUM_USERS = 3;

const int PIN_LENGTH = 4;
const int HISTORY_SIZE = 5;

// 4-digit PINs per user
int userPins[NUM_USERS][PIN_LENGTH] = {
  {1,2,3,4},
  {4,3,2,1},
  {0,0,0,0}
};

// UI state machine modes
enum Mode { MODE_USERSEL, MODE_PIN, MODE_MENU, MODE_DEPOSIT, MODE_WITHDRAW, MODE_HISTORY, MODE_LOCKDOWN };
Mode mode = MODE_USERSEL;

struct ATMState {

  // ---------------- USERS ----------------
  int activeUser;
  int userBalanceCents[NUM_USERS];

  // ---------------- COINS ----------------
  int stockRed;
  int stockGreen;
  int stockBlue;

  // ---------------- MODE ----------------
  Mode mode;

  // ---------------- WITHDRAW ----------------
  int withdrawTargetCents;
  int stepIndex;

  // ---------------- HOLD (WITHDRAW CONFIRM) ----------------
  int checkHoldStartMs;
  bool didHoldConfirm;

  // ---------------- PIN ----------------
  int enteredPin[PIN_LENGTH];
  int pinCursor;
  int currentDigit;
  int pinAttempts;
  bool pinLocked;
  int pinLockStartMs;

  // ---------------- BUTTONS ----------------
  bool bCheck, bLeft, bRight;
  bool checkPressed, leftPressed, rightPressed;
  bool prevCheck, prevLeft, prevRight;

  // ---------------- GENERAL HOLD ----------------
  int checkHoldStart;
  bool holdTriggered;

  // ---------------- LOCKDOWN ----------------
  bool tamperLocked;
  int tamperStartMs;
  int unlockStart;
  int lastFlashMs;
  bool flashOn;

  // ---------------- HISTORY ----------------
  std::string history[HISTORY_SIZE];
  int historyIndex;
  int historyCount;
  bool historyDrawn;

  // ---------------- TIMING ----------------
  int lastInteractionMs;

  // ---------------- SETTINGS ----------------
  int MIN_TIME_BETWEEN_COUNTS_MS;
};

void resetPinEntry(ATMState &s) {
  for(int i = 0; i < PIN_LENGTH; i++) {
    s.enteredPin[i] = -1;
  }
  s.pinCursor = 0;
  s.currentDigit = 0;
}

bool verifyPin(ATMState &s, int userIndex) {
  for(int i = 0; i < PIN_LENGTH; i++) {
    if(s.enteredPin[i] != userPins[userIndex][i]) {
      return false;
    }
  }
  return true;
}

void showPinScreen(ATMState &s) {
  Brain.Screen.clearScreen();

  Brain.Screen.setCursor(1,1);
  Brain.Screen.print("ENTER PIN");

  Brain.Screen.setCursor(3,1);
  Brain.Screen.print("[ ");

  for(int i = 0; i < PIN_LENGTH; i++) {
    if(i < s.pinCursor) {
      Brain.Screen.print("* ");
    }
    else if(i == s.pinCursor) {
      Brain.Screen.print("%d ", s.currentDigit);
    }
    else {
      Brain.Screen.print("_ ");
    }
  }

  Brain.Screen.print("]");

  Brain.Screen.setCursor(5,1);
  Brain.Screen.print("L/R Change  Chk Next");
}

//------------------------------------------------------------------------------
// Small utility functions (math + button helpers)
//------------------------------------------------------------------------------
double absd(double x) { 
  if (x < 0)
    return -x;
  else
    return x;
}                 // Local double abs (avoid cmath abs overload issues)

int minInt(int a, int b) {
  if (a < b)
    return a;
  else
    return b;
}

// risingEdge(): returns true only on the transition false->true
// Used to make buttons act like "click once" instead of repeating while held.
bool risingEdge(bool now, bool &prev) {
  bool edge = (now && !prev);
  prev = now;
  return edge;
}

// currentStepCents(): maps stepIndex to the increment/decrement amount in withdraw screen
int currentStepCents(ATMState &s) {
  if (s.stepIndex == 0) return BLUE_CENTS;
  if (s.stepIndex == 1) return GREEN_CENTS;
  return RED_CENTS;
}

// printMoneyAt(): consistent "$d.dd" formatting in the VEX screen
void printMoneyAt(int row, int col, int cents) {
  Brain.Screen.setCursor(row, col);
  Brain.Screen.print("$%d.%02d", cents/100, cents%100);
}

//------------------------------------------------------------------------------
// UI rendering functions (each clears/redraws the screen for its mode)
//------------------------------------------------------------------------------
void showUserSelectScreen(ATMState &s) {
  Brain.Screen.clearScreen();

  Brain.Screen.setCursor(1,1);
  Brain.Screen.print("SELECT USER");

  Brain.Screen.setCursor(2,1);
  Brain.Screen.print("L/R choose");

  Brain.Screen.setCursor(3,1);
  Brain.Screen.print("User: %d", s.activeUser + 1);

  Brain.Screen.setCursor(4,1);
  Brain.Screen.print("Bal:");
  printMoneyAt(4, 6, s.userBalanceCents[s.activeUser]);

  Brain.Screen.setCursor(5,1);
  Brain.Screen.print("Chk=Select");
}

void showMenuScreen(ATMState &s) {

  Brain.Screen.clearScreen();
    // ----------------------------
    //          Main Menu
    // ----------------------------
  Brain.Screen.setCursor(1,1);
  Brain.Screen.print("MAIN MENU");

  Brain.Screen.setCursor(2,1);
  Brain.Screen.print("Balance: $%.2f",
      s.userBalanceCents[s.activeUser] / 100.0);

  Brain.Screen.setCursor(4,1);
  Brain.Screen.print("Chk=Withdraw");

  Brain.Screen.setCursor(5,1);
  Brain.Screen.print("L=Deposit  R=History");
}

void showDepositScreen(ATMState &s) {
  Brain.Screen.clearScreen();

  Brain.Screen.setCursor(1,1);
  Brain.Screen.print("DEPOSIT");

  Brain.Screen.setCursor(2,1);
  Brain.Screen.print("Insert coin at sensor");

  Brain.Screen.setCursor(3,1);
  Brain.Screen.print("Coins R:%d G:%d", s.stockRed, s.stockGreen);

  Brain.Screen.setCursor(4,1);
  Brain.Screen.print("Coins B:%d", s.stockBlue);

  Brain.Screen.setCursor(5,1);
  Brain.Screen.print("Left=Back");
}

void showWithdrawScreen(ATMState &s) {
  int step = currentStepCents(s);

  Brain.Screen.clearScreen();

  Brain.Screen.setCursor(1,1);
  Brain.Screen.print("WITHDRAW");

  Brain.Screen.setCursor(2,1);
  Brain.Screen.print("Amt:");
  printMoneyAt(2, 6, s.withdrawTargetCents);

  Brain.Screen.setCursor(3,1);
  Brain.Screen.print("Step:");
  printMoneyAt(3, 6, step);

  // Controls:
  // Right adds, Left subtracts, Check cycles step size, Hold Check dispenses
  Brain.Screen.setCursor(4,1);
  Brain.Screen.print("R:+ L:- Chk=Step");

  Brain.Screen.setCursor(5,1);
  Brain.Screen.print("Hold Chk=Disp");
}

// Deposit screen helpers: update only the changing lines (less flicker)
void updateDepositCoinLines(ATMState &s) {
  // Line 3: R and G
  Brain.Screen.setCursor(3,1);
  Brain.Screen.print("Coins R:%d G:%d      ", s.stockRed, s.stockGreen);

  // Line 4: B
  Brain.Screen.setCursor(4,1);
  Brain.Screen.print("Coins B:%d           ", s.stockBlue);
}

// Deposit screen helper: show short status text (success/error)
void showDepositStatus(const char* msg) {
  Brain.Screen.setCursor(2,1);
  Brain.Screen.print("%s                    ", msg); // padded to clear leftovers
}

//------------------------------------------------------------------------------
// Safety / warning UI (hazard icon + lockdown flashing)
//------------------------------------------------------------------------------

// Draw a hazard triangle with an exclamation mark (simple warning icon)
void drawHazardIcon(int x, int y, int size) {
  // Triangle outline
  Brain.Screen.drawLine(x, y + size, x + size, y + size);
  Brain.Screen.drawLine(x + size, y + size, x + size / 2, y);
  Brain.Screen.drawLine(x + size / 2, y, x, y + size);

  // Exclamation mark
  int centerX = x + size / 2;
  Brain.Screen.drawLine(centerX, y + size / 3, centerX, y + (2 * size) / 3);
  Brain.Screen.printAt(centerX - 3, y + (4 * size) / 5, "!");
}

// Flash the hazard + message for a given duration (used for errors/jams)
void flashHazardMessage(const char* msg, int durationMs = 3000) {
  int start = Brain.timer(msec);
  bool visible = true;

  while (Brain.timer(msec) - start < durationMs) {
    Brain.Screen.clearScreen();

    if (visible) {
      drawHazardIcon(20, 20, 80);

      Brain.Screen.setCursor(1, 1);
      Brain.Screen.print("!!! WARNING !!!");

      Brain.Screen.setCursor(6, 3);
      Brain.Screen.print(msg);
    }

    visible = !visible;
    wait(300, msec);   // flash speed
  }

  Brain.Screen.clearScreen();
}

// Lockdown screen: alternates between visible/blank frames to "flash"
void drawLockdownFlashing(bool visible) {
  Brain.Screen.clearScreen();

  if (!visible) {
    // blank flash frame
    return;
  }

  drawHazardIcon(5, 5, 40);

  Brain.Screen.setCursor(1, 8);
  Brain.Screen.print("LOCKDOWN");

  Brain.Screen.setCursor(2, 6);
  Brain.Screen.print("TAMPER DETECTED");

  Brain.Screen.setCursor(4, 1);
  Brain.Screen.print("Hold Check to unlock");

  Brain.Screen.setCursor(5, 1);
  Brain.Screen.print("2 seconds");
}

// checkTamper(): if tilted beyond threshold long enough, force MODE_LOCKDOWN and stop motors
void checkTamper(ATMState &s) {
  int now = Brain.timer(msec);

  double pitch = BrainInertial.pitch(degrees);
  double roll  = BrainInertial.roll(degrees);

  bool tilted = (absd(pitch) > TILT_DEG) || (absd(roll) > TILT_DEG);

  double ax = BrainInertial.acceleration(xaxis);
  double ay = BrainInertial.acceleration(yaxis);
  double az = BrainInertial.acceleration(zaxis);

  double totalAccel = sqrt(ax*ax + ay*ay + az*az);
  bool shaken = (totalAccel > 1.6);  

  bool tamperCondition = tilted || shaken;

  if (tamperCondition) {
    if (s.tamperStartMs < 0) s.tamperStartMs = now;

    if (!s.tamperLocked && (now - s.tamperStartMs) > TAMPER_HOLD_MS) {
      s.tamperLocked = true;
      s.mode = MODE_LOCKDOWN;

      Motor1.stop(brake);
      Motor2.stop(brake);
      Motor6.stop(brake);
    }
  } else {
    s.tamperStartMs = -1;
  }
}

//------------------------------------------------------------------------------
// Coin dispensing (motors) with jam detection
// Each function spins its motor ~360 degrees while monitoring current + movement.
// If jam is detected, it stops and shows a warning.
//------------------------------------------------------------------------------

bool dispenseCoin(motor &m, const char* name, double moveThreshold) {
  const double JAM_CURRENT = 2.0;
  const int JAM_NO_MOVE_MS = 250;
  const int IGNORE_START_MS = 150;
  const int TIMEOUT_MS = 2500;

  int startTime = Brain.timer(msec);
  int noMoveStart = -1;

  m.resetPosition();
  m.spin(forward, 30, percent);

  double lastPos = m.position(degrees);

  while (m.position(degrees) < 360) {
    int now = Brain.timer(msec);

    if (now - startTime > TIMEOUT_MS) {
      m.stop(coast);
      flashHazardMessage(name);
      return false;
    }

    double pos = m.position(degrees);
    double dPos = pos - lastPos;
    lastPos = pos;

    double I = m.current(amp);

    if (now - startTime > IGNORE_START_MS && I > JAM_CURRENT && dPos < moveThreshold) {
      if (noMoveStart < 0) noMoveStart = now;

      if (now - noMoveStart > JAM_NO_MOVE_MS) {
        m.stop(coast);
        flashHazardMessage(name);
        return false;
      }
    } else {
      noMoveStart = -1;
    }

    wait(20, msec);
  }

  m.stop(brake);
  return true;
}

bool dispenseRed(ATMState &s) {
  s.lastInteractionMs = Brain.timer(msec);
  return dispenseCoin(Motor1, "RED JAM - Clear path!", 0.5);
}

bool dispenseGreen(ATMState &s) {
  s.lastInteractionMs = Brain.timer(msec);
  return dispenseCoin(Motor6, "GREEN JAM - Clear path!", 1.0);
}

bool dispenseBlue(ATMState &s) {
  s.lastInteractionMs = Brain.timer(msec);
  return dispenseCoin(Motor2, "BLUE JAM - Clear path!", 1.0);
}

// Adds a transaction to history
void addHistory(ATMState &s, const std::string &msg) {
  s.history[s.historyIndex] = msg;
  s.historyIndex = (s.historyIndex + 1) % HISTORY_SIZE;
  if (s.historyCount < HISTORY_SIZE)
    s.historyCount++;
}

void showHistoryScreen(ATMState &s) {

  Brain.Screen.clearScreen();

  Brain.Screen.setCursor(1,1);
  Brain.Screen.print("TRANSACTION LOG");

  int start = s.historyIndex - s.historyCount;
  if(start < 0) start += HISTORY_SIZE;

  for(int i = 0; i < s.historyCount && i < 4; i++) {

    int index = (start + i) % HISTORY_SIZE;

    Brain.Screen.setCursor(i+2,1);
    Brain.Screen.print(s.history[index].c_str());
  }

  Brain.Screen.setCursor(6,1);
  Brain.Screen.print("Left=Back");
}

//------------------------------------------------------------------------------
// Withdraw planning + execution
// 1) computePlan() decides how many of each coin to use given available stock
// 2) withdrawAmount() validates balance, computes plan, dispenses coins, updates data
//------------------------------------------------------------------------------
bool computePlan(ATMState &s, int amount, int &rOut, int &gOut, int &bOut) {
  rOut = gOut = bOut = 0;

  // Must be positive and a multiple of 5 cents
  if (amount <= 0) return false;
  if (amount % BLUE_CENTS != 0) return false;

  // Try to use higher value coins first (greedy-ish, but with backtracking for green)
  int rMax = minInt(s.stockRed, amount / RED_CENTS);

  for (int r = rMax; r >= 0; --r) {
    int remAfterR = amount - r * RED_CENTS;

    int gMax = minInt(s.stockGreen, remAfterR / GREEN_CENTS);

    for (int g = gMax; g >= 0; --g) {
      int remAfterG = remAfterR - g * GREEN_CENTS;

      if (remAfterG % BLUE_CENTS != 0) continue;

      int b = remAfterG / BLUE_CENTS;

      // Valid plan if we have enough blue coins
      if (b <= s.stockBlue) {
        rOut = r; //r = number of red coins to dispense
        gOut = g; //g = number of green coins to dispense
        bOut = b; //b = number of blue coins to dispense
        return true;
      }
    }
  }

  return false;
}

bool withdrawAmount(ATMState &s, int amountCents) {
  // Validate request
  if (amountCents <= 0) return false;

  // Must have enough user balance
  if (amountCents > s.userBalanceCents[s.activeUser]) {
    flashHazardMessage("INSUFFICIENT BALANCE");
    return false;
  }

  // Must be representable with current stock
  int r=0, g=0, b=0;
  if (!computePlan(s, amountCents, r, g, b)) {
    flashHazardMessage("CANNOT MAKE AMOUNT");
    return false;
  }

  // Dispense in safe order (high -> low), updating stock and user balance each coin
  for (int i = 0; i < r; i++) {
    if (!dispenseRed(s)) { flashHazardMessage("RED JAM"); return false; }
    s.stockRed--;
    s.userBalanceCents[s.activeUser] -= RED_CENTS;
  }

  for (int i = 0; i < g; i++) {
    if (!dispenseGreen(s)) { flashHazardMessage("GREEN JAM"); return false; }
    s.stockGreen--;
    s.userBalanceCents[s.activeUser] -= GREEN_CENTS;
  }

  for (int i = 0; i < b; i++) {
    if (!dispenseBlue(s)) { flashHazardMessage("BLUE JAM"); return false; }
    s.stockBlue--;
    s.userBalanceCents[s.activeUser] -= BLUE_CENTS;
  }

  // Record transaction in history AFTER successful withdrawal
  char msg[32];
  snprintf(msg, 31, "Withdrew $%d.%02d",
           amountCents / 100, amountCents % 100);
  addHistory(s, msg);

  return true;
}

//------------------------------------------------------------------------------
// Deposit coin detection (optical sensor)
// detectCoinCents(): returns coin value if near + recognized color, else 0
// updateBalanceFromCoins(): debounces so one coin only counts once until removed
//------------------------------------------------------------------------------
int detectCoinCents(optical &opt) {

  if (!opt.isNearObject()) return 0;

  double hue = opt.hue();
  double bright = opt.brightness();

  // Ignore very dark readings
  if (bright < 5) return 0;

  // RED
  if (hue > 350 || hue < 30)
    return RED_CENTS;

  // GREEN
  if (hue > 70 && hue < 160)
    return GREEN_CENTS;

  // BLUE
  if (hue > 210 && hue < 270)
    return BLUE_CENTS;

  return 0;
}

void processDeposit(ATMState &s, int cents, const char* msg, int &stock) {
  s.userBalanceCents[s.activeUser] += cents;
  stock++;
  addHistory(s, msg);
  showDepositStatus(msg);
  updateDepositCoinLines(s);
  s.lastInteractionMs = Brain.timer(msec);
}


void depositCoin(ATMState &s, int cents) {
  if (cents == RED_CENTS) {
    s.stockRed++;
    addHistory(s,"Deposit: $1.00");
  }
  else if (cents == GREEN_CENTS) {
    s.stockGreen++;
    addHistory(s,"Deposit: $0.25");
  }
  else if (cents == BLUE_CENTS) {
    s.stockBlue++;
    addHistory(s,"Deposit: $0.05");
  }

  s.userBalanceCents[s.activeUser] += cents;
}

void updateBalanceFromCoins(ATMState &s, optical &opt) {
  // coinLocked prevents double-counting while a coin stays in front of the sensor.
  // lastCountMs is a secondary debounce in case the sensor flickers.
  static bool coinLocked = false;
  static int  lastCountMs = -100000;

  int  now = Brain.timer(msec);
  bool coinPresent = opt.isNearObject();

  // When the coin is removed, unlock so the next coin can be counted.
  if (!coinPresent) coinLocked = false;

  // Count a coin only if:
  // 1) a coin is present
  // 2) we are not locked (meaning we haven't counted this coin yet)
  // 3) sufficient time has passed since last count
  if (coinPresent && !coinLocked && (now - lastCountMs) > s.MIN_TIME_BETWEEN_COUNTS_MS) {
    int cents = detectCoinCents(opt);

  if (cents > 0) {
   depositCoin(s, cents);
  }
    else {
      // Coin present but color doesn't match expected denominations
      showDepositStatus("Unknown coin!");

      s.lastInteractionMs = Brain.timer(msec);
    }

    // Lock until the coin is removed (prevents infinite counting)
    lastCountMs = now;
    coinLocked = true;
  }
}

//------------------------------------------------------------------------------
// main(): initialization + forever loop running the UI state machine
//------------------------------------------------------------------------------

// These functions are to make main more readable:

// USER SELECT
void updateUserSelect(ATMState &s) {
  if (s.rightPressed) {
    s.activeUser = (s.activeUser + 1) % NUM_USERS;
    showUserSelectScreen(s);
  }
  else if (s.leftPressed) {
    s.activeUser = (s.activeUser + NUM_USERS - 1) % NUM_USERS;
    showUserSelectScreen(s);
  }
  else if (s.checkPressed) {
    resetPinEntry(s);
    s.mode = MODE_PIN;
    showPinScreen(s);
  }
}

// PIN
void updatePin(ATMState &s) {
  int now = Brain.timer(msec);

  if (s.pinLocked) {
    int elapsed = now - s.pinLockStartMs;

    if (elapsed >= 10000) {
      s.pinLocked = false;
      s.pinAttempts = 0;
      resetPinEntry(s);
      showPinScreen(s);
    } else {
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(2,1);
      Brain.Screen.print("ACCOUNT LOCKED");
      Brain.Screen.setCursor(3,1);
      Brain.Screen.print("Wait %d sec", (10000 - elapsed)/1000);
    }
    return;
  }

  if (s.rightPressed) {
    s.currentDigit = (s.currentDigit + 1) % 10;
    showPinScreen(s);
  }

  if (s.leftPressed) {
    s.currentDigit--;
    if (s.currentDigit < 0) s.currentDigit = 9;
    showPinScreen(s);
  }

  if (s.checkPressed) {
    s.enteredPin[s.pinCursor] = s.currentDigit;
    s.pinCursor++;
    s.currentDigit = 0;

    if (s.pinCursor >= PIN_LENGTH) {

      if (verifyPin(s, s.activeUser)) {
        Brain.Screen.clearScreen();
        Brain.Screen.setCursor(2,1);
        Brain.Screen.print("PIN ACCEPTED");
        wait(800, msec);

        s.mode = MODE_MENU;
        showMenuScreen(s);
      }
      else {
        s.pinAttempts++;

        Brain.Screen.clearScreen();
        Brain.Screen.setCursor(2,1);
        Brain.Screen.print("INCORRECT PIN");
        Brain.Screen.setCursor(3,1);
        Brain.Screen.print("Attempt %d/3", s.pinAttempts);
        wait(1200, msec);

        if (s.pinAttempts >= 3) {
          s.pinLocked = true;
          s.pinLockStartMs = now;
        }

        resetPinEntry(s);
        showPinScreen(s);
      }
    }
    else {
      showPinScreen(s);
    }
  }
}

// MENU
void updateMenu(ATMState &s) {
  if (s.checkPressed) {
    s.mode = MODE_WITHDRAW;
    s.withdrawTargetCents = 0;
    s.stepIndex = 0;
    showWithdrawScreen(s);
  }
  else if (s.leftPressed) {
    s.mode = MODE_DEPOSIT;
    showDepositScreen(s);
  }
  else if (s.rightPressed) {
    s.mode = MODE_HISTORY;
    s.historyDrawn = false;
    showHistoryScreen(s);
  }
}

// DEPOSIT
void updateDeposit(ATMState &s) {
  updateBalanceFromCoins(s, Optical3);

  if (s.leftPressed) {
    s.mode = MODE_MENU;
    showMenuScreen(s);
  }
}

// WITHDRAW
void updateWithdraw(ATMState &s) {
  int step = currentStepCents(s);

  if (s.rightPressed) {
    s.withdrawTargetCents += step;
    if (s.withdrawTargetCents > s.userBalanceCents[s.activeUser])
      s.withdrawTargetCents = s.userBalanceCents[s.activeUser];
    showWithdrawScreen(s);
  }

  if (s.leftPressed) {
    s.withdrawTargetCents -= step;
    if (s.withdrawTargetCents < 0)
      s.withdrawTargetCents = 0;
    showWithdrawScreen(s);
  }

  if (s.checkPressed) {
    s.stepIndex = (s.stepIndex + 1) % 3;
    showWithdrawScreen(s);
  }

  int now = Brain.timer(msec);

  if (s.bCheck) {
    if (s.checkHoldStart < 0) {
      s.checkHoldStart = now;
      s.holdTriggered = false;
    }
    else if (!s.holdTriggered && (now - s.checkHoldStart) > 1000) {
      s.holdTriggered = true;

      if (s.withdrawTargetCents > 0) {
        withdrawAmount(s, s.withdrawTargetCents);
      }

      s.mode = MODE_MENU;
      showMenuScreen(s);
    }
  } else {
    s.checkHoldStart = -1;
    s.holdTriggered = false;
  }
}

// LOCKDOWN
void updateLockdown(ATMState &s) {
  int now = Brain.timer(msec);

  // Flashing
  if (now - s.lastFlashMs >= 300) {
    s.lastFlashMs = now;
    s.flashOn = !s.flashOn;
    drawLockdownFlashing(s.flashOn);
  }

  // Hold to unlock
  if (s.bCheck) {
    if (s.unlockStart < 0) s.unlockStart = now;

    Brain.Screen.setCursor(3, 1);
    Brain.Screen.print("Unlock %d/%d   ", (now - s.unlockStart), UNLOCK_HOLD_MS);

    if (now - s.unlockStart >= UNLOCK_HOLD_MS) {
      s.tamperLocked = false;
      s.tamperStartMs = -1;
      s.unlockStart = -1;
      s.checkHoldStart = -1;
      s.holdTriggered = false;

      s.mode = MODE_USERSEL;
      showUserSelectScreen(s);
    }
  } else {
    s.unlockStart = -1;
  }
}

//HISTORY
void updateHistory(ATMState &s) {
  if (s.leftPressed) {
    s.mode = MODE_MENU;
    showMenuScreen(s);
  }
}

void readButtons(ATMState &s) {
  s.bCheck = Brain.buttonCheck.pressing();
  s.bLeft  = Brain.buttonLeft.pressing();
  s.bRight = Brain.buttonRight.pressing();

  s.checkPressed = risingEdge(s.bCheck, s.prevCheck);
  s.leftPressed  = risingEdge(s.bLeft, s.prevLeft);
  s.rightPressed = risingEdge(s.bRight, s.prevRight);
}

//KILL SWITCH!!!
bool shouldExit() {
  return Bumper4.pressing();
}

int main() {
  vexcodeInit();

  ATMState s;

  // Users
  s.activeUser = 0;
  s.userBalanceCents[0] = 500;
  s.userBalanceCents[1] = 10000;
  s.userBalanceCents[2] = 5;
  
  // Coins
  s.stockRed = 10;
  s.stockGreen = 10;
  s.stockBlue = 10;
  
  // Mode
  s.mode = MODE_USERSEL;
  s.withdrawTargetCents = 0;
  s.stepIndex = 0;
  
  // PIN
  s.pinCursor = 0;
  s.currentDigit = 0;
  s.pinAttempts = 0;
  s.pinLocked = false;
  
  // Buttons
  s.prevCheck = s.prevLeft = s.prevRight = false;
  
  // Hold
  s.checkHoldStart = -1;
  s.holdTriggered = false;
  
  // Lockdown
  s.tamperLocked = false;
  s.tamperStartMs = -1;
  s.unlockStart = -1;
  s.lastFlashMs = 0;
  s.flashOn = true;
  
  // History
  s.historyIndex = 0;
  s.historyCount = 0;
  s.historyDrawn = false;
  
  // Timing
  s.lastInteractionMs = 0;
  
  // Settings
  s.MIN_TIME_BETWEEN_COUNTS_MS = 350;

  s.withdrawTargetCents = 0;
  s.stepIndex = 0;

  s.checkHoldStartMs = -1;
  s.didHoldConfirm = false;
  
  // Calibrate inertial FIRST so pitch/roll readings are valid for tamper detection
  BrainInertial.calibrate();
  while (BrainInertial.isCalibrating()) {
    Brain.Screen.clearScreen();
    Brain.Screen.setCursor(2, 1);
    Brain.Screen.print("Calibrating...");
    wait(50, msec);
  }

  // Configure optical sensor for reliable color readings
  Optical3.setLightPower(100);
  Optical3.setLight(ledState::on);
  checkTamper(s);
  // Start in user select mode
  s.mode = MODE_USERSEL;
  showUserSelectScreen(s);
  s.lastInteractionMs = Brain.timer(msec);

  while (!shouldExit()) {
  
    readButtons(s);
    checkTamper(s);
  
    if (s.mode == MODE_LOCKDOWN)
    updateLockdown(s);
    else if (s.mode == MODE_USERSEL)
    updateUserSelect(s);
    else if (s.mode == MODE_PIN)
    updatePin(s);
    else if (s.mode == MODE_MENU)
    updateMenu(s);
    else if (s.mode == MODE_DEPOSIT)
    updateDeposit(s);
    else if (s.mode == MODE_WITHDRAW)
    updateWithdraw(s);
    else if (s.mode == MODE_HISTORY)
    updateHistory(s);

    wait(20, msec);
  }

  Brain.Screen.clearScreen();
  wait(500, msec);

  Brain.programStop();
  return 0;
}
