KMyMoney2 Overview. 2 March 2001.


Introduction.

The design of this project is to create a simple personal finances manager
for KDE2 aimed for the home user.  Business use is supported but not all
of the typical business needs will be catered for within the foreseeable
future.

Before I describe what KMyMoney2 will be able to do when it reaches maturity
I will give a little background on why I started this project in the first
place which will explain the more fundamental design decisions.


Background.

When I first started university a year and a half ago I searched the internet
for a simple to use but functional personal finances manager that would run
on my Linux machine hoping that I could just use Windows for certain games.
After much searching I found several programs which were of all good quality
but early in development.  Continuing to use MS-Money I thought that it would
be nice to have a customised app that could do the things I wanted such as
weekly analysation of the transactions I made.  I had taught myself c a few
year earlier to program games under MS-DOS and decided to create a customised
money clone for myself that would run under Linux and my favourite window
manager of the time FVWM, (which came with Slackware!).  Work begun but was
hindered by me having to learn X programming, C++ and Linux programming in
general all at once.  I then happended upon Mandrake and the wonderful KDE 1.
Development moved over to KDE within a week and things started moving.  Then
KDE2 came along and I made the necessary changes to port to KDE2, (quite a task),
and start using KDevelop.  During the port I had the unsuccesful idea of renaming
KMyMoney to KMyMoney2 which would indicate a new version but the name had stuck
and I was left with it.  (KMyMoney started life as a temporary name until I
could think of something better, the K being for KDE and MyMoney because I
intended to emulate MS-Money!).

One and a half years later KMyMoney2 is not finished, is quite far off from being
finished and I have a wealth of features, (planned or started), that were not
in the original plan.  This project was not intended for general use and had
to be retrofitted for general consumption hence the slow release time due to
design issues I had dissmissed early in the life of KMyMoney.  Combine that with
my lack of experience with Linux, C++, QT and KDE we reach where we are now :- a
functional but limited personal finances application runnning under KDE2 that is
currently under development.  However, compared to other much larger projects such
as GNU Cash, work is progressing nicely and the first stable version should be released
within the year.


Design and Goals.

As mentioned in the Background i first started this project to keep track of my
finances whilst at University.  Original goals were simple but functional for
what I needed:

  A register view that can display all the transactions or a user defined section.
  A way to store information about the banks and accounts that I interact with.
  Simple reporting features so I can see how I am spending my money.

Simple but effecient!

Because of these simple goals I ignored such requirements as currency support and
internationalisation.  I also only included the account types that I use - savings
and current, a 'feature' still present in its present incarnation.
Due to the fact that I only wanted transaction information no other types of 'account'
were designed either, these include accounts such as assets, stocks and credit cards etc.

So what will KMyMoney2 provide in it's first stable version?

Hopefully all the above along with an applet that will monitor your finances whilst
running on the taskbar and will alert you when a bill is overdue or automatically
create a recurring transaction such as a wage payment or bill payment.

Full currency support is high on the priority list and will be implemented as soon
as the features started have matured.


The future and beyond!

Any ideas about the future of KMyMoney2 are really appreciated and any code submitted
will be used with my great thanks!

As an aside I am a useless artist and am even worse on the computer.  If anyone can create
icons and pictures or even give hints on the appearance of KMyMoney2 that would also be
appreciated.


Finality.

Some api documentation is located in the subdirectories from the current directory and should
be read before reading the source files to give an overview of what each module does.  Hopefully
at some point near in the future some HTML API documentation should be produced from the source
files which will give a clear idea of how KMyMoney2 works.


Michael...
mte@users.sourceforge.net