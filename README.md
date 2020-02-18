# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore
- Latest build status with azerothcore: [![Build Status](https://travis-ci.org/azerothcore/mod-ah-bot.svg?branch=master)](https://travis-ci.org/azerothcore/mod-ah-bot)
# Mod-AHBOT


## Description

An auction house bot for the best core: AzerothCore.


## Installation

```
1. Simply place the module under the `modules` directory of your AzerothCore source. 
1. Import the SQL manually to the right Database (auth, world or characters) or with the `db_assembler.sh` (if `include.sh` provided).
1. Re-run cmake and launch a clean build of AzerothCore.
```

## Edit module configuration (optional)

If you need to change the module configuration, go to your server configuration folder (where your `worldserver` or `worldserver.exe` is)
rename the file mod_ahbot.conf.dist to mod_ahbot.conf and edit it.


## Usage

Edit the module configuration and add a player account ID and a character ID.
This character will sell and buy items in the auction house so give him a good name.
The account used does not need any security level and can be a player account.


## Credits

- Ayase: ported the bot to AzerothCore
- Other contributors (check the contributors list)

