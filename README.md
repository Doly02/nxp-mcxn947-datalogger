## Datalogger With Recording on SD Card And Prevention of Data Loss In Case of Power Failure 

- Author: Tomáš Dolák 
- Login: [xdolak09](https://www.vut.cz/lide/tomas-dolak-247220)
- Email: <xdolak09@stud.fit.vutbr.cz>
<br>

- Supervisor: Martin Moštěk 

The Goal of This Project Is To Developt Datalogger That Will Record Datalogs From UART Peripheral And Store Them 
on The SD Card, And With Prevention of Data Loss in Case of Power Failure.   

### Table of Contents
- [Requirements](#requirements)
    - [Hardware Components](#hardware-components)
- [Repository Organization](#repository-organization)
- [How To Use The LaTeX Project?](#how-to-use-the-latex-project)
    - [Potential Problem](#potential-problem)


### Requirements 
To build and run `datalogger`, you will need the following:

#### Hardware Components
1. NXP FRDM-MCXN947 Development Board
2. DS3231 Real-Time Circuit 

#### Software Requirements
1. MCUXpresso IDE v11.10.0

More Will Be Add Soon!

### Repository Organization
```
/nxp-mcxn947-datalogger
├── /software
│ ├── /source    
│ │    ├── main.c
│ └── /include 
│      ├── data_types.h
│
└── /thesis (Bachelor Thesis Text Submodule)
  ├── main_page.tex
  ├── /pics
  └── make
```

### How To Use The LaTeX Project?

On Ubuntu (or Any Other Linux System e.g. WSL) Download Complete LaTeX Distribution (Recommended):

```
sudo apt-get update
sudo apt-get install texlive-full
```

Or Download Download Just The LaTeX Core:
```
sudo apt-get update
sudo apt-get install texlive-latex-base
```

Check The Correctness of Installation:
```
which pdflatex
```

Try To Build The LaTeX Project By:
```
cd /thesis
make clean
make
```

#### Potential Problem
If The `project.pfd` File Does Not Exist in The `/thesis` Folder, The Program Has Not Compiled Correctly, Probably Because of Missing Packages. In This Case Install Followed LaTex Packages:

```
sudo apt-get install texlive-lang-czechslovak
```

```
sudo apt-get install texlive-latex-recommended
```

```
sudo apt-get install texlive-latex-extra
```

```
sudo apt-get install texlive-fonts-extra
```