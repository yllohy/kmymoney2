Transaction Engine Overview. 2 March 2001.

The files in this directory make up the engine of the transactional code.
Each file describes a certain part of the engine and is used to query that
object.  It will hopefully be, some time in the future, GUI independant but
currently contains code that links it to the QT library.  When this code is
removed it is quite feasible to use the engine under other GUIs such as GNOME
and Windows.

The main interface to the engine is done through the MyMoneyFile class which
represents the file located in permanent storage either over the web or on a local
hard disk.  MyMoneyFile contains code to read and write the file to disk along with
methods to store and query banks which are described later.  Future modifications
to the MyMoneyFile class will create new classes for creating different account types
such as assets, stocks, credit cards etc.

MyMoneyFile stores information about all the banks that the user interacts with.  A bank
is a representation of a physical high street bank and has such attributes as accounts which
contain transactions.  The MyMoneyBank class represents this bank and contains methods to
add and query all of its accounts.  The MyMoneybank class, at the moment, reflects a typical
UK bank with the sort code and address fields stored as attributes.

MyMoneyAccount represents an account within the engine, again focusing on the UK
idea of an account.  An account can, at the moment, be either a savings or current account
with no facilities to create other types.  When new account types are created modifications
will need to be made to some of the code within MyMoneyAccount to facilitate non-standard
account types.

An account stores its transactions in a list which can then be searched and interrogated.
A transaction is represented by the MyMoneyTransaction class which stores its amount attribute
as a MyMoneyMoney which currently just stores a double to represent pounds and pence (UK - GBP).
Each transaction can be one of 5 different types which indicate a typical transaction, exmaples
being a cheque transaction or deposit transaction.  These are currently processed internally
and have no way of being updated and no way for new types to be added e.g Direct Debit which
you would have to classify as Withdrawal currently.  To faciliate reconciliation within the
engine each transaction also stores its state to be queried at any time.

For information on how the code works or how the transaction engine is queried please look
at the source files for further information and/or the API documentation (when it gets
built!).