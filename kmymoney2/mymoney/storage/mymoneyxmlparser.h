/***************************************************************************
                          mymoneyxmlparser.h  -  description
                             -------------------
    begin                : Mon Nov 11 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYXMLPARSER_H
#define MYMONEYXMLPARSER_H

#include <xml++.h>

/**class that is a custom wrapper around libxml++.
  *@author Kevin Tambascio
  */

class XMLParserCallback;

class MyMoneyXMLParser
{
public:

  void  setParserCallback(xmlpp::XMLParserCallback* pParser)
  {
    m_pParserCallback = pParser;
  }

  
  
private:
  xmlpp::XMLParserCallback *m_pParserCallback;
  xmlParserCtxtPtr _context;

  static xmlEntityPtr _get_entity(void *_parser, const xmlChar *n) {
    return xmlGetPredefinedEntity(n);
  };

  static void _start_document(void *_parser) {
    MyMoneyXMLParser *parser;

    parser = (MyMoneyXMLParser *) _parser;
    parser->start_document();
  };

  static void _end_document(void *_parser) {
    MyMoneyXMLParser *parser;

    parser = (MyMoneyXMLParser *) _parser;
    parser->end_document();
  };

  static void _start_element(void *_parser, const xmlChar *n,
				const xmlChar **p) {
    MyMoneyXMLParser *parser;
    xmlpp::XMLPropertyMap properties;

    if(p) {
      const xmlChar **cur;

      for(cur = p; cur && *cur; cur++) {
        std::string name, value;

        name = std::string((const char *) *cur++);
        value = std::string((const char *) *cur);

        properties[name] = new xmlpp::XMLProperty(name, value);
      }
    }

    parser = (MyMoneyXMLParser *) _parser;
    parser->start_element(std::string((const char *) n), properties);
  };

  static void _end_element(void *_parser, const xmlChar *n) {
    MyMoneyXMLParser *parser;

    parser = (MyMoneyXMLParser *) _parser;
    parser->end_element(std::string((const char *) n));
  };

  static void _characters(void *_parser, const xmlChar *s, int len) {
    MyMoneyXMLParser *parser;

    parser = (MyMoneyXMLParser *) _parser;
    parser->characters(std::string((const char *) s, len));
  };

  static void _comment(void *_parser, const xmlChar *s) {
    MyMoneyXMLParser *parser;

    parser = (MyMoneyXMLParser *) _parser;
    parser->comment(std::string((const char *) s));
  };

  static void _warning(void *_parser, const char *fmt, ...) {
    MyMoneyXMLParser *parser;
    va_list arg;
    char buff[1024];

    va_start(arg, fmt);
    vsprintf(buff, fmt, arg);
    va_end(arg);

    parser = (MyMoneyXMLParser *) _parser;
    parser->warning(std::string(buff));
  };

  static void _error(void *_parser, const char *fmt, ...) {
    MyMoneyXMLParser *parser;
    va_list arg;
    char buff[1024];

    va_start(arg, fmt);
    vsprintf(buff, fmt, arg);
    va_end(arg);

    parser = (MyMoneyXMLParser *) _parser;
    parser->error(std::string(buff));
  };

  static void _fatal_error(void *_parser, const char *fmt, ...) {
    MyMoneyXMLParser *parser;
    va_list arg;
    char buff[1024];

    va_start(arg, fmt);
    vsprintf(buff, fmt, arg);
    va_end(arg);

    parser = (MyMoneyXMLParser *) _parser;
    parser->fatal_error(std::string(buff));
  };

  void start_document(void) {
    if(m_pParserCallback)
      m_pParserCallback->start_document();
  };

  void end_document(void) {
    if(m_pParserCallback)
      m_pParserCallback->end_document();
  };

  void start_element(const std::string &n, const xmlpp::XMLPropertyMap &p) {
    if(m_pParserCallback)
      m_pParserCallback->start_element(n, p);
  };

  void end_element(const std::string &n) {
    if(m_pParserCallback)
      m_pParserCallback->end_element(n);
  };

  void characters(const std::string &s) {
    if(m_pParserCallback)
      m_pParserCallback->characters(s);
  };

  void comment(const std::string &s) {
    if(m_pParserCallback)
      m_pParserCallback->comment(s);
  };

  void warning(const std::string &s) {
    if(m_pParserCallback)
      m_pParserCallback->warning(s);
  };

  void error(const std::string &s) {
    if(m_pParserCallback)
      m_pParserCallback->error(s);
  };

  void fatal_error(const std::string &s) {
    if(m_pParserCallback)
      m_pParserCallback->fatal_error(s);
  };
public:
  MyMoneyXMLParser() {
    xmlSAXHandler sax_handler = {
      NULL,		// internalSubset
      NULL,		// isStandalone
      NULL,		// hasInternalSubset
      NULL,		// hasExternalSubset
      NULL,		// resolveEntity
      _get_entity,	// getEntity
      NULL,		// entityDecl
      NULL,		// notationDecl
      NULL,		// attributeDecl
      NULL,		// elementDecl
      NULL,		// unparsedEntityDecl
      NULL,		// setDocumentLocator
      _start_document,	// startDocument
      _end_document,	// endDocument
      _start_element,	// startElement
      _end_element,	// endElement
      NULL,		// reference
      _characters,	// characters
      NULL,		// ignorableWhitespace
      NULL,		// processingInstruction
      _comment,		// comment
      _warning,		// warning
      _error,		// error
      _fatal_error,	// fatalError
      NULL,		// getParameterEntity
      NULL,		// cdataBlock
      NULL		// externalSubset
    };

    //_parser_callback = new ParserCallback;
    _context = xmlCreatePushParserCtxt(&sax_handler, this, NULL, 0, NULL);
  };

  ~MyMoneyXMLParser()
  {
    if(_context)
      xmlFreeParserCtxt(_context);
  };

  void parse_chunk(const std::string &s) {
    xmlParseChunk(_context, s.c_str(), s.length(), 0);
  };

  void finish(void) {
    xmlParseChunk(_context, NULL, 0, 1);
  };
    
};

#endif
