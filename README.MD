# ArduinoLock

## Description:
This project is just the barebones. It's an 3X4 Matrix Numpad with an 4 rows x 20 characters display and a servo (optional).

## Options:
- Enable Changing the passcode while the pragram is running
- Enable the servo (only usefull for testing purposes )
- Enable the serial debug (only for test purposes or nerds :-) )

## Explanation:

The code is pretty difficult when you don't split it into some parts:

- The variable declarations
- The initialisation of libraries
- The functions that are used by other functions
- The functions that are used in the main loop 

### How does the code work?
- First we create all our variables (A lot at first but we use them all)
- Seccond we initialise all our libraries
    - The keypad library requires us to give the Keymap (Where every key is positioned), how many rows and colloms there are and where the pins are connected to
    - The LCD library requires us to give some stuff too
    - The servo library requires us to create an attribute for our new servo
- Functions: 
    - The initialisation of the LCD
        Now to start the program we want to test if the lcd is fully functional by:  
        - Enabeling, disabeling and reanabeling the backlight
        - Showing the start message on the screen
    - The ability to disable backlight afther 30 secconds
    - Getting the pressed key
    - Getting the code from user input
    - Check the code if it's correct or not
    - Open or close the lock
    - Read and write the stored code to eeprom memory
    - Getting a new code while the program is running
    - Comparing the to given codes and see if they match
- Setup:
    - Getting the eeprom code
    - When enabled initialize the servo library to attach to the servo
    - Initialize the lcd
    - When enabled initialize the Serial communication
- Main Loop:
    - Getting the keypress
    - When you are in the unlocked state and you press * the lock is going to close
    - When you are in the unlocked state and you press # the lock will enter "Change Passcode" state
    - Open the lock when the LockState in true same as Opened (Pretty straigth foreward isn't it)
    - Or getting user input to get the code.