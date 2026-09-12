# ME101-ATM
# ME101 ATM Machine

> A fully functional ATM system designed, programmed, and manufactured as part of a first-year mechanical engineering design project at the University of Waterloo.

The project combines **C++ programming** with a **VEX IQ-based electromechanical system** to simulate the operation of an ATM, including user authentication, deposits, withdrawals, automated coin dispensing, transaction history, and tamper detection.

---

## Project Overview

The ATM uses three types of coins:

| Colour   | Value |
| -------- | ----: |
| 🔴 Red   | $1.00 |
| 🟢 Green | $0.25 |
| 🔵 Blue  | $0.05 |

Users can:

* Select an account
* Enter a four-digit PIN
* View their account balance
* Deposit coins
* Withdraw money
* View recent transaction history

An **optical sensor** identifies deposited coins based on their colour, while three motors automatically dispense coins during withdrawals.

---

## Key Features

* 👤 Multi-user account system
* 🔐 Four-digit PIN authentication
* 🔒 Account lockout after repeated incorrect PIN attempts
* 👁️ Optical colour sensing for coin identification
* ⚙️ Automated coin dispensing
* 🧮 Withdrawal planning based on available coin inventory
* 📋 Transaction history
* 🛠️ Motor jam detection using current and position feedback
* 🚨 Inertial-based tamper detection
* 🔒 Automatic lockdown when tampering is detected
* 🛑 Emergency kill switch
* 🖥️ VEX IQ Brain-based user interface

---

## Technical Implementation

### C++

The program uses several C++ concepts, including:

* Structs
* Enumerations
* Functions
* Arrays
* References
* `std::string`
* State-machine logic
* Sensor and motor control
* Timing and event detection

### State Machine

The ATM operates through several program states:

```text
User Selection
      ↓
    PIN
      ↓
  Main Menu
   ↙  ↓  ↘
Deposit Withdraw History
```

A separate **Lockdown** state is triggered when the inertial sensor detects excessive tilt or acceleration.

---

## Withdrawal System

The withdrawal system determines whether a requested amount can be produced using the ATM's available coin inventory.

The program accounts for:

* User balance
* Available $1.00 coins
* Available $0.25 coins
* Available $0.05 coins

Once a valid combination is determined, the corresponding motors dispense the required coins and the user's balance and coin inventory are updated.

---

## Jam Detection

During coin dispensing, the program monitors both **motor position** and **motor current**.

If a motor draws excessive current without sufficient movement for a specified period, the system identifies a potential jam and:

1. Stops the motor
2. Displays a warning
3. Prevents the system from continuing the dispensing operation

This provides basic fault detection and improves the reliability of the mechanical system.

---

## Tamper Detection

The ATM uses an **inertial sensor** to monitor pitch, roll, and acceleration.

If abnormal movement persists bey
