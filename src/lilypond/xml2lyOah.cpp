/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include <sstream>

#include <climits>      // INT_MIN, INT_MAX
#include <iomanip>      // setw, setprecision, ...

#include "libmusicxml.h"

#include "xml2lyOah.h"

#include "generalOah.h"

#include "setTraceOahIfDesired.h"
#include "traceOah.h"

#include "executableOah.h"
#include "musicXMLOah.h"
#include "msrOah.h"
#include "lpsrOah.h"
#include "lilypondOah.h"

#include "extraOah.h"

#include "version.h"
#include "msr.h"
#include "lpsr.h"


using namespace std;

namespace MusicXML2
{

//______________________________________________________________________________
S_xml2lyVersionOahAtom xml2lyVersionOahAtom::create (
  string shortName,
  string longName,
  string description)
{
  xml2lyVersionOahAtom* o = new
    xml2lyVersionOahAtom (
      shortName,
      longName,
      description);
  assert(o!=0);
  return o;
}

xml2lyVersionOahAtom::xml2lyVersionOahAtom (
  string shortName,
  string longName,
  string description)
  : oahAtom (
      shortName,
      longName,
      description)
{}

xml2lyVersionOahAtom::~xml2lyVersionOahAtom ()
{}

S_oahValuedAtom xml2lyVersionOahAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // handle it at once
  printVersion (os);

  // no option value is needed
  return nullptr;
}

void xml2lyVersionOahAtom::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyVersionOahAtom::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyVersionOahAtom>*
    p =
      dynamic_cast<visitor<S_xml2lyVersionOahAtom>*> (v)) {
        S_xml2lyVersionOahAtom elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyVersionOahAtom::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void xml2lyVersionOahAtom::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyVersionOahAtom::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyVersionOahAtom>*
    p =
      dynamic_cast<visitor<S_xml2lyVersionOahAtom>*> (v)) {
        S_xml2lyVersionOahAtom elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyVersionOahAtom::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void xml2lyVersionOahAtom::browseData (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyVersionOahAtom::browseData ()" <<
      endl;
  }
#endif
}

void xml2lyVersionOahAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "xml2lyVersionOahAtom:" <<
    endl;

  gIndenter++;

  oahElement::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  gIndenter--;
}

void xml2lyVersionOahAtom::printVersion (ostream& os) const
{
  os <<
    endl <<
    "This is xml2ly" <<
    " version " << currentVersionNumber () <<
    endl <<
    endl;

  // print versions history
  printVersionsHistory (os);
}

void xml2lyVersionOahAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // nothing to print here
}

ostream& operator<< (ostream& os, const S_xml2lyVersionOahAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_xml2lyAboutOahAtom xml2lyAboutOahAtom::create (
  string shortName,
  string longName,
  string description)
{
  xml2lyAboutOahAtom* o = new
    xml2lyAboutOahAtom (
      shortName,
      longName,
      description);
  assert(o!=0);
  return o;
}

xml2lyAboutOahAtom::xml2lyAboutOahAtom (
  string shortName,
  string longName,
  string description)
  : oahAtom (
      shortName,
      longName,
      description)
{}

xml2lyAboutOahAtom::~xml2lyAboutOahAtom ()
{}

S_oahValuedAtom xml2lyAboutOahAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // handle it at once
  printAbout (os);

  // no option value is needed
  return nullptr;
}

void xml2lyAboutOahAtom::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyAboutOahAtom::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyAboutOahAtom>*
    p =
      dynamic_cast<visitor<S_xml2lyAboutOahAtom>*> (v)) {
        S_xml2lyAboutOahAtom elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyAboutOahAtom::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void xml2lyAboutOahAtom::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyAboutOahAtom::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyAboutOahAtom>*
    p =
      dynamic_cast<visitor<S_xml2lyAboutOahAtom>*> (v)) {
        S_xml2lyAboutOahAtom elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyAboutOahAtom::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void xml2lyAboutOahAtom::browseData (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyAboutOahAtom::browseData ()" <<
      endl;
  }
#endif
}

void xml2lyAboutOahAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "xml2lyAboutOahAtom:" <<
    endl;

  gIndenter++;

  oahElement::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  gIndenter--;
}

void xml2lyAboutOahAtom::printAbout (ostream& os) const
{
  os <<
    endl <<
R"(What xml2ly does:

    This multi-pass translator basically performs 5 passes:
        Pass 1:  reads the contents of MusicXMLFile or stdin ('-')
                 and converts it to a MusicXML tree;
        Pass 2a: converts that MusicXML tree into to
                 a Music Score Representation (MSR) skeleton;
        Pass 2b: converts that tree and the skeleton into a
                 Music Score Representation (MSR);
        Pass 3:  converts the MSR into a
                 LilyPond Score Representation (LPSR);
        Pass 4:  converts the LPSR to LilyPond source code
                 and writes it to standard output.

    Other passes are performed according to the options, such as
    printing views of the internal data or printing a summary of the score.

    The activity log and warning/error messages go to standard error.)" <<
    endl;
}

void xml2lyAboutOahAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // nothing to print here
}

ostream& operator<< (ostream& os, const S_xml2lyAboutOahAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_xml2lyContactOahAtom xml2lyContactOahAtom::create (
  string shortName,
  string longName,
  string description)
{
  xml2lyContactOahAtom* o = new
    xml2lyContactOahAtom (
      shortName,
      longName,
      description);
  assert(o!=0);
  return o;
}

xml2lyContactOahAtom::xml2lyContactOahAtom (
  string shortName,
  string longName,
  string description)
  : oahAtom (
      shortName,
      longName,
      description)
{}

xml2lyContactOahAtom::~xml2lyContactOahAtom ()
{}

S_oahValuedAtom xml2lyContactOahAtom::handleOptionUnderName (
  string   optionName,
  ostream& os)
{
  // handle it at once
  printContact (os);

  // no option value is needed
  return nullptr;
}

void xml2lyContactOahAtom::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyContactOahAtom::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyContactOahAtom>*
    p =
      dynamic_cast<visitor<S_xml2lyContactOahAtom>*> (v)) {
        S_xml2lyContactOahAtom elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyContactOahAtom::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void xml2lyContactOahAtom::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyContactOahAtom::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyContactOahAtom>*
    p =
      dynamic_cast<visitor<S_xml2lyContactOahAtom>*> (v)) {
        S_xml2lyContactOahAtom elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyContactOahAtom::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void xml2lyContactOahAtom::browseData (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyContactOahAtom::browseData ()" <<
      endl;
  }
#endif
}

void xml2lyContactOahAtom::print (ostream& os) const
{
  const int fieldWidth = K_OPTIONS_FIELD_WIDTH;

  os <<
    "xml2lyContactOahAtom:" <<
    endl;

  gIndenter++;

  oahElement::printOptionEssentials (
    os, fieldWidth);

  gIndenter++;
  os <<
    gIndenter.indentMultiLineString (
      fDescription) <<
    endl;
  gIndenter--;

  gIndenter--;
}

void xml2lyContactOahAtom::printContact (ostream& os) const
{
  os <<
    endl <<
R"(To contact xml2ly maintainers:

    Send a mail to mailto:lilypond-user@gnu.org describing the problem
    and error messages you obtain if relevant.)" <<
    endl;
}

void xml2lyContactOahAtom::printAtomOptionsValues (
  ostream& os,
  int      valueFieldWidth) const
{
  // nothing to print here
}

ostream& operator<< (ostream& os, const S_xml2lyContactOahAtom& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_xml2lyOahHandler xml2lyOahHandler::create (
  string           executableName,
  indentedOstream& ios)
{
  xml2lyOahHandler* o = new
    xml2lyOahHandler (
      executableName,
      ios);
  assert(o!=0);
  return o;
}

xml2lyOahHandler::xml2lyOahHandler (
  string           executableName,
  indentedOstream& ios)
  : oahHandler (
    "Available options",
    "Options values",
    "h", "help",
    "hs", "helpSummary",
R"(
                      Welcome to xml2ly,
              the MusicXML to LilyPond translator
          delivered as part of the libmusicxml2 library.
      https://github.com/grame-cncm/libmusicxml/tree/lilypond

Usage:
    xml2ly [options] [MusicXMLFile|-] [options]
)",
R"(
Option '-h, -help' prints the full help,
  while '-hs, -helpSummary' only prints a help summary.)",
    ios)
{
  // create and append the help options prefixes
  S_oahPrefix
    helpPrefix =
      oahPrefix::create (
        "help",
        "help-",
        "'-help=abc,xywx-yz' is equivalent to '-help-abc, -help-xywx-yz'");
  appendPrefixToHandler (helpPrefix);

  S_oahPrefix
    hPrefix =
      oahPrefix::create (
        "h",
        "h",
        "'-h=abc,wxyz' is equivalent to '-habc, -hwxyz'");
  appendPrefixToHandler (hPrefix);

  // create and append the trace options prefixes
  S_oahPrefix
    tracePrefix =
      oahPrefix::create (
        "trace",
        "trace-",
        "'-trace=abc,xywx-yz' is equivalent to '-trace-abc, -trace-xywx-yz'");
  appendPrefixToHandler (tracePrefix);

  S_oahPrefix
    tPrefix =
      oahPrefix::create (
        "t",
        "t",
        "'-t=abc,wxyz' is equivalent to '-tabc, -twxyz'");
  appendPrefixToHandler (tPrefix);

  // initialize the handler only now, since it may use prefixes
  initializeOptionsHandler (executableName);
}

xml2lyOahHandler::~xml2lyOahHandler ()
{}

void xml2lyOahHandler::initializeOptionsHandler (
  string executableName)
{
  // initialize options handling, phase 1
  // ------------------------------------------------------

#ifdef TRACE_OAH
  initializeTraceOahHandling (
    this);
#endif

  initializeExecutableOahHandling (
    executableName,
    this);

  initializeGeneralOahHandling (
    this);

  // initialize the library
  // ------------------------------------------------------

  initializeMSR ();
  initializeLPSR ();

  // initialize options handling, phase 2
  // ------------------------------------------------------

  initializeMusicXMLOahHandling (
    this);

  initializeMsrOahHandling (
    this);

  initializeLpsrOahHandling (
    this);

  initializeLilypondOahHandling (
    this);

#ifdef EXTRA_OAH
  initializeExtraOahHandling (
    this);
#endif

  initializeXml2lyOah (
    this);

#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah && ! gGeneralOah->fQuiet) {
    // print the options handler initial state
    fHandlerLogOstream <<
      "xml2lyOahHandler has been initialized as:" <<
      endl;

    gIndenter++;

    print (
      fHandlerLogOstream);
    fHandlerLogOstream <<
      endl <<
      endl;

    gIndenter--;
  }
#endif

  // register options handler in itself,
  // so that the 'global' help options can be handled
  this->
    registerHandlerInItself ();

#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah && ! gGeneralOah->fQuiet) {
    fHandlerLogOstream <<
      "xml2lyOahHandler help:" <<
      endl;

    this->
      printHelp (
        fHandlerLogOstream);
  }
#endif
}

void xml2lyOahHandler::checkOptionsAndArguments ()
{
  unsigned int argumentsNumber =
    fHandlerArgumentsVector.size ();

#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah && ! gGeneralOah->fQuiet) {
    if (argumentsNumber > 0) {
      fHandlerLogOstream <<
        singularOrPluralWithoutNumber (
          argumentsNumber, "There is", "There are") <<
        " " <<
        argumentsNumber <<
        " " <<
        singularOrPluralWithoutNumber (
          argumentsNumber, "argument", "arguments") <<
        ":" <<
        endl;

      gIndenter++;

      for (unsigned int i = 0; i < argumentsNumber; i++) {
        fHandlerLogOstream <<
          i << " : " << fHandlerArgumentsVector [i] <<
            endl;
      } // for

      fHandlerLogOstream << endl;

      gIndenter--;
    }
    else {
      fHandlerLogOstream <<
        "There are no arguments to " <<
        gExecutableOah->fHandlerExecutableName <<
        endl;
    }
  }
#endif

  // input source name
  // ------------------------------------------------------

  switch (argumentsNumber) {
    case 0:
      fHandlerLogOstream <<
        endl <<
        "Input file name or '-' for standard input expected" <<
        endl <<
        endl;

      exit (1);
      break;

    case 1:
      // register intput file name
      gExecutableOah->fInputSourceName =
        fHandlerArgumentsVector [0];
      break;

    default:
      fHandlerLogOstream <<
        endl <<
        "Several input file name supplied, only the first one, \"" <<
        fHandlerArgumentsVector [0] <<
        "\", will be translated" <<
        endl <<
        endl;

      // register intput file name
      gExecutableOah->fInputSourceName =
        fHandlerArgumentsVector [0];
      break;
  } //  switch

  // build potential output file name
  // ------------------------------------------------------

  string potentialOutputFileName;

  if (gExecutableOah->fInputSourceName != "-") {
    // determine potential output file name,
    // may be set differently by '--ofn, --outputFileName' option
    potentialOutputFileName =
      baseName (
        gExecutableOah->fInputSourceName);

    // set '.ly' suffix
    size_t
      posInString =
        potentialOutputFileName.rfind ('.');

    if (posInString != string::npos) {
      potentialOutputFileName.replace (
        posInString,
        potentialOutputFileName.size () - posInString,
        ".ly");
    }
  }

  // check auto output file option usage
  // ------------------------------------------------------

  if (gXml2lyOah->fAutoOutputFileName) {
    if (gXml2lyOah->fOutputFileName.size ()) {
      stringstream s;

      s <<
        "options '-aofn, -auto-output-file-name' and '-o, -output-file-name'"  <<
        endl <<
        "cannot be chosen simultaneously";

      oahError (s.str ());

      exit (3);
    }

    else if (gExecutableOah->fInputSourceName == "-") {
      stringstream s;

      s <<
        "option '-aofn, -auto-output-file-name'"  <<
        endl <<
        "cannot be used when reading from standard input";

      oahError (s.str ());

      exit (4);
    }

    else {
      gXml2lyOah->fOutputFileName =
        potentialOutputFileName;
    }
  }

  // register command line informations in gGeneralOah
  // ------------------------------------------------------

/* JMI ???
  gGeneralOah->fCommandLineWithShortOptions =
    fCommandLineWithShortOptions;
  gGeneralOah->fCommandLineWithShortOptions =
    fCommandLineWithShortOptions;
    */
}

//______________________________________________________________________________
void xml2lyOahHandler::enforceOahHandlerQuietness ()
{
#ifdef TRACE_OAH
  gGeneralOah->
    enforceQuietness ();
#endif

  gGeneralOah->
    enforceQuietness ();

  gMusicXMLOah->
    enforceQuietness ();

  gMsrOah->
    enforceQuietness ();

  gLpsrOah->
    enforceQuietness ();

  gLilypondOah->
    enforceQuietness ();

#ifdef EXTRA_OAH
  gExtraOah->
    enforceQuietness ();
#endif

  gXml2lyOah->
    enforceQuietness ();
}

//______________________________________________________________________________
void xml2lyOah::enforceQuietness ()
{}

//______________________________________________________________________________
void xml2lyOah::checkOptionsConsistency ()
{
  gLogOstream <<
    "xml2lyOah::checkOptionsConsistency ()" <<
    endl;

#ifdef TRACE_OAH
  gGeneralOah->
    checkOptionsConsistency ();
#endif
  gGeneralOah->
    checkOptionsConsistency ();

  gMusicXMLOah->
    checkOptionsConsistency ();

  gMsrOah->
    checkOptionsConsistency ();

  gLpsrOah->
    checkOptionsConsistency ();

  gLilypondOah->
    checkOptionsConsistency ();

#ifdef EXTRA_OAH
  gExtraOah->
    checkOptionsConsistency ();
#endif

/* JMI TICINO
  gXml2lyOah->
    checkOptionsConsistency ();
    */
}

//______________________________________________________________________________
void xml2lyOah::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyOah::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyOah>*
    p =
      dynamic_cast<visitor<S_xml2lyOah>*> (v)) {
        S_xml2lyOah elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyOah::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void xml2lyOah::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyOah::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyOah>*
    p =
      dynamic_cast<visitor<S_xml2lyOah>*> (v)) {
        S_xml2lyOah elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyOah::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void xml2lyOah::browseData (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyOah::browseData ()" <<
      endl;
  }
#endif
}

//______________________________________________________________________________
void xml2lyOahHandler::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyOahHandler::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyOahHandler>*
    p =
      dynamic_cast<visitor<S_xml2lyOahHandler>*> (v)) {
        S_xml2lyOahHandler elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyOahHandler::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void xml2lyOahHandler::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyOahHandler::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_xml2lyOahHandler>*
    p =
      dynamic_cast<visitor<S_xml2lyOahHandler>*> (v)) {
        S_xml2lyOahHandler elem = this;

#ifdef TRACE_OAH
        if (gExecutableOah->fTraceOahVisitors) {
          gLogOstream <<
            "% ==> Launching xml2lyOahHandler::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void xml2lyOahHandler::browseData (basevisitor* v)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOahVisitors) {
    gLogOstream <<
      "% ==> xml2lyOahHandler::browseData ()" <<
      endl;
  }
#endif
}

//______________________________________________________________________________
void xml2lyOahHandler::print (ostream& os) const
{
  const int fieldWidth = 27;

  os <<
    "xml2lyOahHandler:" <<
    endl;

  gIndenter++;

  os << left <<
    setw (fieldWidth) <<
    "fShortName" << " : " << fShortName <<
    endl <<
    setw (fieldWidth) <<
    "fLongName" << " : " << fLongName <<
    endl <<
    setw (fieldWidth) <<
    "fDescription" << " : " << fDescription <<
    endl <<
    endl;

  os <<
    "Options groups (" <<
    singularOrPlural (
      fHandlerGroupsList.size (), "element",  "elements") <<
    "):" <<
    endl;

  if (fHandlerGroupsList.size ()) {
    os << endl;

    gIndenter++;

    list<S_oahGroup>::const_iterator
      iBegin = fHandlerGroupsList.begin (),
      iEnd   = fHandlerGroupsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      // print the element
      os << (*i);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }

  gIndenter--;

  os << endl;
}

ostream& operator<< (ostream& os, const S_xml2lyOahHandler& elt)
{
  elt->print (os);
  return os;
}

//_______________________________________________________________________________
S_xml2lyOah gXml2lyOah;

S_xml2lyOah xml2lyOah::create (
  S_oahHandler handlerUpLink)
{
  xml2lyOah* o = new xml2lyOah (
    handlerUpLink);
  assert(o!=0);

  return o;
}

xml2lyOah::xml2lyOah (
  S_oahHandler handlerUpLink)
  : oahGroup (
    "xml2ly",
    "hxl", "help-xml2ly",
R"(Options that are used by xml2ly are grouped here.)",
    kElementVisibilityAlways,
    handlerUpLink)
{
  // append this options group to the options handler
  // if relevant
  if (handlerUpLink) {
    handlerUpLink->
      appendGroupToHandler (this);
  }

  // initialize it
  initializeXml2lyOah ();
}

xml2lyOah::~xml2lyOah ()
{}

void xml2lyOah::initializeXml2lyOah ()
{
  // version
  // --------------------------------------

  {
    S_oahSubGroup
      versionSubGroup =
        oahSubGroup::create (
          "Version",
          "hxv", "help-xml2ly-version",
R"()",
        kElementVisibilityAlways,
        this);

    appendSubGroupToGroup (versionSubGroup);

    // version

    versionSubGroup->
      appendAtomToSubGroup (
        xml2lyVersionOahAtom::create (
          "v", "version",
R"(Display xml2ly's version number and history and exit.)"));
  }


  // about
  // --------------------------------------

  {
    S_oahSubGroup
      aboutSubGroup =
        oahSubGroup::create (
          "About",
          "hxa", "help-xml2ly-about",
R"()",
        kElementVisibilityAlways,
        this);

    appendSubGroupToGroup (aboutSubGroup);

    // about

    aboutSubGroup->
      appendAtomToSubGroup (
        xml2lyAboutOahAtom::create (
          "a", "about",
R"(Display information about xml2ly and exit.)"));
  }


  // contact
  // --------------------------------------

  {
    S_oahSubGroup
      contactSubGroup =
        oahSubGroup::create (
          "Contact",
          "hxc", "help-xml2ly-contact",
R"()",
        kElementVisibilityAlways,
        this);

    appendSubGroupToGroup (contactSubGroup);

    // contact

    contactSubGroup->
      appendAtomToSubGroup (
        xml2lyContactOahAtom::create (
          "c", "contact",
R"(Display information about how to contacct xml2ly maintainers and exit.)"));
  }


  // output file
  // --------------------------------------

  {
    S_oahSubGroup
      outputFileSubGroup =
        oahSubGroup::create (
          "Output file",
          "hxof", "help-xml2ly-output-file",
R"()",
        kElementVisibilityAlways,
        this);

    appendSubGroupToGroup (outputFileSubGroup);

    // output filename

    outputFileSubGroup->
      appendAtomToSubGroup (
        oahStringAtom::create (
          "o", "output-file-name",
R"(Write LilyPond code to file FILENAME instead of standard output.)",
          "FILENAME",
          "outputFileName",
          fOutputFileName));

    // auto output filename

    fAutoOutputFileName = false;

    outputFileSubGroup->
      appendAtomToSubGroup (
        oahBooleanAtom::create (
          "aofn", "auto-output-file-name",
R"(This option can only be used when reading from a file.
Write LilyPond code to a file in the current working directory.
The file name is derived from that of the input file,
replacing any suffix after the the '.' by 'ly'
or adding '.ly' if none is present.)",
          "autoOutputFileName",
          fAutoOutputFileName));
  }
}

void xml2lyOah::printXml2lyOahValues (int fieldWidth)
{
  gLogOstream <<
    "The xml2ly options are:" <<
    endl;

  gIndenter++;

  // output file
  // --------------------------------------

  gLogOstream << left <<
    setw (fieldWidth) << "Output file:" <<
    endl;

  gIndenter++;

  gLogOstream << left <<
    setw (fieldWidth) << "outputFileName" << " : \"" <<
    fOutputFileName <<
    "\"" <<
    endl;

  gIndenter--;
}

//______________________________________________________________________________
void initializeXml2lyOah (
  S_oahHandler handler)
{
#ifdef TRACE_OAH
  if (gExecutableOah->fTraceOah && ! gGeneralOah->fQuiet) {
    gLogOstream <<
      "Initializing xml2ly options handling" <<
      endl;
  }
#endif

  // enlist versions information
  // ------------------------------------------------------

  enlistVersion (
    "Initial", "early 2016",
    "Start as xml2lilypond, a clone of xml2guido");

  enlistVersion (
    musicxml2lilypondVersionStr (), "October 2018",
    "First draft version");

  // create the options variables
  // ------------------------------------------------------

  gXml2lyOah = xml2lyOah::create (
    handler);
  assert (gXml2lyOah != 0);
}


}
