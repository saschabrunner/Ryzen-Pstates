# Ryzen Pstates

## Description
A simple command line tool to update Pstates on Zen CPUs on Windows.


## Building

### Dependencies
* Visual Studio

### Build
Open the project and build it in Visual Studio.

There is some support for GCC, but it hasn't been tested.

## Support
Every Zen, Zen+ and Zen 2 CPU with less than 64 threads (32 cores) should be supported.

**So far it has only been tested on a Raven Ridge CPU (2500U) though!**

The code has some safety measures to stop you from applying dangerous settings but no guarantees are made.

**Use this at your own risk!**

## Usage

### Example
`ryzen_pstates -p=1 -f=90 -d=10 -v=88`

### Options
```
-p, --pstate    Required, Selects PState to change (0 - 7)
-f, --fid       New FID to set (16 - 255)
-d, --did       New DID to set (8 - 26)
-v, --vid       New VID to set (32 - 168)
--dry-run       Only display current and calculated new pstate, but don't apply it
```

### Screenshot
![Screenshot](https://i.imgur.com/CGmRdx5.png)
