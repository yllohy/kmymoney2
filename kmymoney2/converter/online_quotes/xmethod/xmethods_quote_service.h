/*  Web Services Definitions net.xmethods.services.stockquote.StockQuote */

/*  Modify this file to customize the generated data type declarations */

/*

**  The gSOAP WSDL parser for C and C++ 1.1.2
**  Copyright (C) 2001-2004 Robert van Engelen, Genivia, Inc.
**  All Rights Reserved. This product is provided "as is", without any warranty.


--------------------------------------------------------------------------------
gSOAP XML Web services tools
Copyright (C) 2001-2004, Robert van Engelen, Genivia, Inc. All Rights Reserved.

GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------
*/

//gsoapopt cw

/* Service net_x002exmethods_x002eservices_x002estockquote_x002eStockQuoteBinding operations:

  ns1__getQuote

*/


// Service net.xmethods.services.stockquote.StockQuoteService : net.xmethods.services.stockquote.StockQuote web service
//gsoap ns1 service name:	XMethodQuotes
//gsoap ns1 service type:	net_x002exmethods_x002eservices_x002estockquote_x002eStockQuotePortType 
//gsoap ns1 service port:	http://64.124.140.30:9090/soap 
//gsoap ns1 service namespace:	urn:xmethods-delayed-quotes 

/* Service net_x002exmethods_x002eservices_x002estockquote_x002eStockQuoteBinding operation ns1__getQuote

  C stub function (defined in soapClient.c[pp]):
  int soap_call_ns1__getQuote(struct soap *soap,
    NULL, (char *endpoint = NULL selects default endpoint for this operation)
    NULL, (char *action = NULL selects default action for this operation)
    char*                               symbol,
  struct ns1__getQuoteResponse {
    float                               Result;
  } * );

*/

//gsoap ns1 service method-style:	getQuote rpc
//gsoap ns1 service method-encoding:	getQuote http://schemas.xmlsoap.org/soap/encoding/
//gsoap ns1 service method-action:	getQuote urn:xmethods-delayed-quotes#getQuote
int ns1__getQuote(
    char*                               _symbol,
  struct ns1__getQuoteResponse {
    float                               _Result;
  } * );

/*  End of Web Services Definitions net.xmethods.services.stockquote.StockQuote */
