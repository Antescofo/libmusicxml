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
#include <algorithm>    // for_each

#include "conversions.h"

#include "msr2LpsrTranslator.h"

#include "generalOptions.h"

#include "setTraceOptionsIfDesired.h"
#ifdef TRACE_OPTIONS
  #include "traceOptions.h"
#endif

#include "musicXMLOptions.h"
#include "msrOptions.h"
#include "lilypondOptions.h"


using namespace std;

namespace MusicXML2
{

//________________________________________________________________________
msr2LpsrTranslator::msr2LpsrTranslator (
  indentedOstream& ios,
  S_msrScore       mScore)
    : fLogOutputStream (ios)
{
  // the MSR score we're visiting
  fVisitedMsrScore = mScore;

  // identification
  fOnGoingIdentification = false;

  // header
  fWorkNumberKnown       = false;
  fWorkTitleKnown        = false;
  fMovementNumberKnown   = false;
  fMovementTitleKnown    = false;
   
  // staves
  fOnGoingStaff = false;

  // voices
  fOnGoingHarmonyVoice     = false;
  fOnGoingFiguredBassVoice = false;

  // repeats

  // notes
  fOnGoingNote = false;

  // double tremolos
  fOnGoingDoubleTremolo = false;

  // grace notes
  fOnGoingGraceNotesGroup = false;
  
  // chords
  fOnGoingChord = false;
  
  // stanzas
  fOnGoingStanza = false;

  // syllables
  fOnGoingSyllableExtend = false;
};
  
msr2LpsrTranslator::~msr2LpsrTranslator ()
{}

//________________________________________________________________________
void msr2LpsrTranslator::buildLpsrScoreFromMsrScore ()
{
  if (fVisitedMsrScore) {    
    // create a msrScore browser
    msrBrowser<msrScore> browser (this);

    // browse the score with the browser
    browser.browse (*fVisitedMsrScore);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::setPaperIndentsIfNeeded (
  S_msrPageGeometry pageGeometry)
{
  S_lpsrPaper
    paper =
      fLpsrScore->getPaper ();

  int
    scorePartGroupNamesMaxLength =
      fCurrentMsrScoreClone->
        getScorePartGroupNamesMaxLength ();

  int
    scorePartNamesMaxLength =
      fCurrentMsrScoreClone->
        getScorePartNamesMaxLength ();

  int
    scoreInstrumentNamesMaxLength =
      fCurrentMsrScoreClone->
        getScoreInstrumentNamesMaxLength ();

  int
    scoreInstrumentAbbreviationsMaxLength =
      fCurrentMsrScoreClone->
        getScoreInstrumentAbbreviationsMaxLength ();

  // compute the maximum value
  int maxValue = -1;
  
  if (scorePartGroupNamesMaxLength > maxValue) {
    maxValue = scorePartGroupNamesMaxLength;
  }
  
  if (scorePartNamesMaxLength > maxValue) {
    maxValue = scorePartNamesMaxLength;
  }
  
  if (scoreInstrumentNamesMaxLength > maxValue) {
    maxValue = scoreInstrumentNamesMaxLength;
  }
  
  // compute the maximum short value
  int maxShortValue = -1;
  
  if (scoreInstrumentAbbreviationsMaxLength > maxShortValue) {
    maxShortValue = scoreInstrumentAbbreviationsMaxLength;
  }

  // heuristics to determine the number of characters per centimeter
  float charactersPerCemtimeter = 4.0;

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceGeometry) {
    // get the paper width
    float paperWidth = pageGeometry->getPaperWidth ();
  
    fLogOutputStream <<
      "setPaperIndentsIfNeeded():" <<
      endl;

    gIndenter++;

    const int fieldWidth = 40;
    
    fLogOutputStream << left <<
      setw (fieldWidth) <<
      "scorePartGroupNamesMaxLength" << " : " <<
      scorePartGroupNamesMaxLength <<
      endl <<
      setw (fieldWidth) <<
      "scorePartNamesMaxLength" << " : " <<
      scorePartNamesMaxLength <<
      endl <<
      setw (fieldWidth) <<
      "scoreInstrumentNamesMaxLength" << " : " <<
      scoreInstrumentNamesMaxLength <<
      endl <<
      setw (fieldWidth) <<
      "scoreInstrumentAbbreviationsMaxLength" << " : " <<
      scoreInstrumentAbbreviationsMaxLength <<
      endl <<
      setw (fieldWidth) <<
      "maxValue" << " : " <<
      maxValue <<
      endl <<
      setw (fieldWidth) <<
      "maxShortValue" << " : " <<
      maxShortValue <<
      endl <<
      setw (fieldWidth) <<
      "paperWidth" << " : " <<
      paperWidth <<
      endl <<
      setw (fieldWidth) <<
      "charactersPerCemtimeter" << " : " <<
      charactersPerCemtimeter <<
      endl;

    gIndenter--;
  }
#endif

  // set indent if relevant
  if (maxValue > 0) {
    paper->
      setIndent (
        maxValue / charactersPerCemtimeter);
  }
  
  // set short indent if relevant
  if (maxShortValue > 0) {
    paper->
      setShortIndent (
        maxShortValue / charactersPerCemtimeter);
  }
}

//________________________________________________________________________
/* JMI
void msr2LpsrTranslator::prependSkipGraceNotesGroupToPartOtherVoices (
  S_msrPart            partClone,
  S_msrVoice           voiceClone,
  S_msrGraceNotesGroup skipGraceNotesGroup)
{
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTraceGraceNotes) {
      fLogOutputStream <<
        "--> prepending a skip graceNotesGroup clone " <<
        skipGraceNotesGroup->asShortString () <<
        " to voices other than \"" <<
        voiceClone->getVoiceName () << "\"" <<
        " in part " <<
        partClone->getPartCombinedName () <<
        ", line " << skipGraceNotesGroup->getInputLineNumber () <<
        endl;
    }
#endif
  
  map<int, S_msrStaff>
    partStavesMap =
      partClone->
        getPartStavesMap ();

  for (
    map<int, S_msrStaff>::const_iterator i=partStavesMap.begin ();
    i!=partStavesMap.end ();
    i++) {

    list<S_msrVoice>
      staffAllVoicesList =
        (*i).second->
          getStaffAllVoicesList ();
          
    for (
      list<S_msrVoice>::const_iterator j=staffAllVoicesList.begin ();
      j!=staffAllVoicesList.end ();
      j++) {

      S_msrVoice voice = (*j);
      
      if (voice != voiceClone) {
        // prepend skip grace notes to voice JMI
        / *
        voice->
          prependGraceNotesGroupToVoice (
            skipGraceNotesGroup);
            * /
      }
    } // for

  } // for
}
*/

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrScore& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrScore" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create an empty clone of fVisitedMsrScore for use by the LPSR score
  // not sharing the visitiged MSR score allows cleaner data handling
  // and optimisations of the LPSR data
  fCurrentMsrScoreClone =
    fVisitedMsrScore->
      createScoreNewbornClone ();

  // create the LPSR score
  fLpsrScore =
    lpsrScore::create (
      NO_INPUT_LINE_NUMBER,
      fCurrentMsrScoreClone);
      
  // fetch score header
  fLpsrScoreHeader =
    fLpsrScore-> getHeader();

  // is there a rights option?
  if (gLilypondOptions->fRights.size ()) {
    // define rights
    
    fLpsrScoreHeader->
      addRights (
        inputLineNumber,
        gLilypondOptions->fRights);
  }

  // is there a composer option?
  if (gLilypondOptions->fComposer.size ()) {
    // define composer
    
    fLpsrScoreHeader->
      addComposer (
        inputLineNumber,
        gLilypondOptions->fComposer);
  }

  // is there an arranger option?
  if (gLilypondOptions->fArranger.size ()) {
    // define arranger
    
    fLpsrScoreHeader->
      addArranger (
        inputLineNumber,
        gLilypondOptions->fArranger);
  }

  // is there a poet option?
  if (gLilypondOptions->fPoet.size ()) {
    // define poet
    
    fLpsrScoreHeader->
      addPoet (
        inputLineNumber,
        gLilypondOptions->fPoet);
  }

  // is there a lyricist option?
  if (gLilypondOptions->fLyricist.size ()) {
    // define lyricist
    
    fLpsrScoreHeader->
      addLyricist (
        inputLineNumber,
        gLilypondOptions->fLyricist);
  }

  // is there a software option?
  if (gLilypondOptions->fSoftware.size ()) {
    // define software
    
    fLpsrScoreHeader->
      addSoftware (
        inputLineNumber,
        gLilypondOptions->fSoftware);
  }

  // is the Scheme function 'whiteNoteHeads' to be generated?
  if (gLilypondOptions->fWhiteNoteHeads) {
    fLpsrScore->
      // this score needs the 'whiteNoteHeads' Scheme function
      setWhiteNoteHeadsIsNeeded ();
  }

  // is Jianpu notation to be generated?
  if (gLilypondOptions->fJianpu) {
    fLpsrScore->
      // this score needs the 'jianpu file include' Scheme function
      setJianpuFileIncludeIsNeeded ();
  }

/* JMI
  // push it onto this visitors's stack,
  // making it the current partGroup block
  fPartGroupBlocksStack.push (
    partGroupBlock);
    */
}

void msr2LpsrTranslator::visitEnd (S_msrScore& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrScore" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  if (fWorkTitleKnown && fMovementTitleKnown) {
    string
      workTitle =
        fCurrentIdentification->
          getWorkTitle ()->
            getVariableValue (),
      movementTitle =
        fCurrentIdentification->
          getMovementTitle ()->
            getVariableValue ();
    
    if (
      workTitle.size () == 0
        &&
      movementTitle.size () > 0) {
      // use the movement title as the work title
      fCurrentIdentification->
        setWorkTitle (
          inputLineNumber, movementTitle);

      fLpsrScoreHeader->
        setWorkTitle (
          inputLineNumber, 
          movementTitle,
          kFontStyleNone,
          kFontWeightNone);

      // forget the movement title
      fCurrentIdentification->
        setMovementTitle (
          inputLineNumber, "");

      fLpsrScoreHeader->
        setMovementTitle (
          inputLineNumber,
          "",
          kFontStyleNone,
          kFontWeightNone);
    }
  }

  else if (! fWorkTitleKnown && fMovementTitleKnown) {
    string
      movementTitle =
        fCurrentIdentification->
          getMovementTitle ()->
            getVariableValue ();
            
    // use the movement title as the work title
    fCurrentIdentification->
      setWorkTitle (
        inputLineNumber, movementTitle);

    fLpsrScoreHeader->
      setWorkTitle (
        inputLineNumber,
        movementTitle,
        kFontStyleNone,
        kFontWeightNone);

    // forget the movement title
    fCurrentIdentification->
      setMovementTitle (
        inputLineNumber, "");

    fLpsrScoreHeader->
      setMovementTitle (
        inputLineNumber,
        "",
        kFontStyleNone,
        kFontWeightNone);
  }

  if (fWorkNumberKnown && fMovementNumberKnown) {
    string
      workNumber =
        fCurrentIdentification->
          getWorkNumber ()->
            getVariableValue (),
      movementNumber =
        fCurrentIdentification->
          getMovementNumber ()->
            getVariableValue ();
    
    if (
      workNumber.size () == 0
        &&
      movementNumber.size () > 0) {
      // use the movement number as the work number
      fCurrentIdentification->
        setWorkNumber (
          inputLineNumber, movementNumber);

      fLpsrScoreHeader->
        setWorkNumber (
          inputLineNumber, 
          movementNumber,
          kFontStyleNone,
          kFontWeightNone);

      // forget the movement number
      fCurrentIdentification->
        setMovementNumber (
          inputLineNumber, "");

      fLpsrScoreHeader->
        setMovementNumber (
          inputLineNumber,
          "",
          kFontStyleNone,
          kFontWeightNone);
    }
  }

  else if (! fWorkNumberKnown && fMovementNumberKnown) {
    string
      movementNumber =
        fCurrentIdentification->
          getMovementNumber ()->
            getVariableValue ();
            
    // use the movement number as the work number
    fCurrentIdentification->
      setWorkNumber (
        inputLineNumber, movementNumber);

    fLpsrScoreHeader->
      setWorkNumber (
        inputLineNumber,
        movementNumber,
        kFontStyleNone,
        kFontWeightNone);

    // forget the movement number
    fCurrentIdentification->
      setMovementNumber (
        inputLineNumber, "");

    fLpsrScoreHeader->
      setMovementNumber (
        inputLineNumber,
        "",
        kFontStyleNone,
        kFontWeightNone);
  }

  // set ident and short indent if needed
  setPaperIndentsIfNeeded (
    elt->getPageGeometry ());
  
/* JMI
  // get top level partgroup block from the stack
  S_lpsrPartGroupBlock
    partGroupBlock =
      fPartGroupBlocksStack.top ();

  // pop it from the stack
  fPartGroupBlocksStack.top ();

  // the stack should now be empty
  if (fPartGroupBlocksStack.size ()) {
    msrInternalError (
      1,
      "the partGroup block stack is not exmpty at the end of the visit");
  }
   */ 
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrIdentification& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrIdentification" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  gIndenter++;

  fCurrentIdentification =
    fLpsrScore->
      getMsrScore ()->
        getIdentification ();
    
  fOnGoingIdentification = true;
}

void msr2LpsrTranslator::visitEnd (S_msrIdentification& elt)
{
  fOnGoingIdentification = false;
  
  gIndenter--;

#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrIdentification" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPageGeometry& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPageGeometry" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  gIndenter++;

  // create a page geometry clone
  S_msrPageGeometry
    pageGeometryClone =
      elt->createGeometryNewbornClone ();

  // register it in current MSR score clone
  fCurrentMsrScoreClone->
    setPageGeometry (
      pageGeometryClone);

  // get LPSR score paper
  S_lpsrPaper
    paper =
      fLpsrScore->getPaper ();

  // populate paper  
  paper ->
    setPaperWidth (elt->getPaperWidth ());
  paper->
    setPaperHeight (elt->getPaperHeight ());
    
  paper->
    setTopMargin (elt->getTopMargin ());
  paper->
    setBottomMargin (elt->getBottomMargin ());
  paper->
    setLeftMargin (elt->getLeftMargin ());
  paper->
    setRightMargin (elt->getRightMargin ());

  // get LPSR score layout
  S_lpsrLayout
    scoreLayout =
      fLpsrScore->getScoreLayout ();

  // get LPSR score global staff size
  float
    globalStaffSize =
      elt->globalStaffSize ();

  // populate layout JMI ???
  /*
  scoreLayout->
    setMillimeters (elt->getMillimeters ());
  scoreLayout->
    setTenths (elt->getTenths ());
    */

  // populate LPSR score global staff size
  fLpsrScore->
    setGlobalStaffSize (globalStaffSize);

  // get LPSR score block layout
  S_lpsrLayout
    scoreBlockLayout =
      fLpsrScore->getScoreBlock ()->getScoreBlockLayout ();

  // create the score block layout staff-size Scheme assoc
  stringstream s;
  s << globalStaffSize;
  S_lpsrSchemeVariable
    assoc =
      lpsrSchemeVariable::create (
        NO_INPUT_LINE_NUMBER, // JMI
        lpsrSchemeVariable::kCommentedYes,
        "layout-set-staff-size",
        s.str (),
        "Uncomment and adapt next line as needed (default is 20)",
        lpsrSchemeVariable::kEndlOnce);

  // populate score block layout
  scoreBlockLayout->
    addSchemeVariable (assoc);

/* JMI
    void    setBetweenSystemSpace (float val) { fBetweenSystemSpace = val; }
    float   getBetweenSystemSpace () const    { return fBetweenSystemSpace; }

    void    setPageTopSpace       (float val) { fPageTopSpace = val; }
   */
}

void msr2LpsrTranslator::visitEnd (S_msrPageGeometry& elt)
{  
  gIndenter--;

#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPageGeometry" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrCredit& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCredit" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentCredit = elt;

  // set elt as credit of the MSR score part of the LPSR score
  fLpsrScore->
    getMsrScore ()->
      appendCreditToScore (fCurrentCredit);
}

void msr2LpsrTranslator::visitEnd (S_msrCredit& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrCredit" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentCredit = nullptr;
}

void msr2LpsrTranslator::visitStart (S_msrCreditWords& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCreditWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // don't append it to the current credit, since the latter is no clone
  /* JMI
  fCurrentCredit->
    appendCreditWordsToCredit (
      elt);
      */
}

void msr2LpsrTranslator::visitEnd (S_msrCreditWords& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrCreditWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPartGroup& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPartGroup " <<
      elt->getPartGroupCombinedName () <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a partGroup clone
  // current partGroup clone, i.e. the top of the stack,
  // is the uplink of the new one if it exists

  S_msrPartGroup
    partGroupClone =
      elt->createPartGroupNewbornClone (
        fPartGroupsStack.size ()
          ? fPartGroupsStack.top ()
          : 0,
        fLpsrScore->getMsrScore ());

  // push it onto this visitors's stack,
  // making it the current partGroup block
#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTracePartGroups) {
    fLogOutputStream <<
      "Pushing part group clone " <<
      partGroupClone->getPartGroupCombinedName () <<
      " onto stack" <<
      endl;
  }
#endif
  
  fPartGroupsStack.push (
    partGroupClone);
  
/*
  // add it to the MSR score clone
  fCurrentMsrScoreClone->
    addPartGroupToScore (fCurrentPartGroupClone);
*/

  // create a partGroup block refering to the part group clone
  S_lpsrPartGroupBlock
    partGroupBlock =
      lpsrPartGroupBlock::create (
        partGroupClone);

  // push it onto this visitors's stack,
  // making it the current partGroup block
  fPartGroupBlocksStack.push (
    partGroupBlock);
  
  // get the LPSR store block
  S_lpsrScoreBlock
    scoreBlock =
      fLpsrScore->getScoreBlock ();

  // don't append the partgroup block to the score block now:
  // this will be done when it gets popped from the stack
}

void msr2LpsrTranslator::visitEnd (S_msrPartGroup& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPartGroup " <<
      elt->getPartGroupCombinedName () <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  S_msrPartGroup
    currentPartGroup =
      fPartGroupsStack.top ();
      
  if (fPartGroupsStack.size () == 1) {
    // add the current partgroup clone to the MSR score clone
    // if it is the top-level one, i.e it's alone in the stack
    
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTracePartGroups) {
      fLogOutputStream <<
        "Adding part group clone " <<
        currentPartGroup->getPartGroupCombinedName () <<
        " to MSR score" <<
        endl;
    }
#endif

    fCurrentMsrScoreClone->
      addPartGroupToScore (currentPartGroup);

    fPartGroupsStack.pop ();
  }

  else {

    // pop current partGroup from this visitors's stack
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTracePartGroups) {
      fLogOutputStream <<
        "Popping part group clone " <<
        fPartGroupsStack.top ()->getPartGroupCombinedName () <<
        " from stack" <<
        endl;
    }
#endif

    fPartGroupsStack.pop ();

    // append the current part group to the one one level higher,
    // i.e. the new current part group
    fPartGroupsStack.top ()->
      appendSubPartGroupToPartGroup (
        currentPartGroup);
  }

  // get the LPSR store block
  S_lpsrScoreBlock
    scoreBlock =
      fLpsrScore->getScoreBlock ();
      
  S_lpsrPartGroupBlock
    currentPartGroupBlock =
      fPartGroupBlocksStack.top ();
      
  if (fPartGroupBlocksStack.size () == 1) {
    // add the current partgroup clone to the LPSR score's parallel music
    // if it is the top-level one, i.e it's alone in the stack
    
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTracePartGroups) {
      fLogOutputStream <<
        "Adding part group block clone for part group " <<
        currentPartGroupBlock->
          getPartGroup ()->
            getPartGroupCombinedName () <<
        " to LPSR score" <<
        endl;
    }
#endif

    // append the current partgroup block to the score block
    // if it is the top-level one, i.e it's alone in the stack
   // JMI BOF if (fPartGroupBlocksStack.size () == 1)
      scoreBlock->
        appendPartGroupBlockToScoreBlock (
          fPartGroupBlocksStack.top ());
          
    // pop current partGroup block from this visitors's stack,
    // only now to restore the appearence order
    fPartGroupBlocksStack.pop ();
  }

  else {
    // pop current partGroup block from this visitors's stack
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTracePartGroups) {
      fLogOutputStream <<
        "Popping part group block clone for part group " <<
        currentPartGroupBlock->
          getPartGroup ()->
            getPartGroupCombinedName () <<
        " from stack" <<
        endl;
    }
#endif

    fPartGroupBlocksStack.pop ();

    // append the current part group block to the one one level higher,
    // i.e. the new current part group block
    fPartGroupBlocksStack.top ()->
      appendElementToPartGroupBlock (
        currentPartGroupBlock);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPart& elt)
{
#ifdef TRACE_OPTIONS
  int inputLineNumber =
    elt->getInputLineNumber ();
#endif
    
  string
    partCombinedName =
      elt->getPartCombinedName ();
      
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPart " <<
      partCombinedName <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceParts) {
    fLogOutputStream <<
      endl <<
      "<!--=== part \"" << partCombinedName << "\"" <<
      ", line " << inputLineNumber << " ===-->" <<
      endl;
  }
#endif

  gIndenter++;

  // create a part clone
  fCurrentPartClone =
    elt->createPartNewbornClone (
      fPartGroupsStack.top ());

  // add it to the partGroup clone
#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceParts) {
    fLogOutputStream <<
      "Adding part clone " <<
      fCurrentPartClone->getPartCombinedName () <<
      " to part group clone \"" <<
      fPartGroupsStack.top ()->getPartGroupCombinedName () <<
      "\"" <<
      endl;
  }
#endif

  fPartGroupsStack.top ()->
    appendPartToPartGroup (fCurrentPartClone);

  // create a part block
  fCurrentPartBlock =
    lpsrPartBlock::create (
      fCurrentPartClone);

  // append it to the current partGroup block
#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceParts) {
    fLogOutputStream <<
      "Appending part block " <<
      fPartGroupsStack.top ()->getPartGroupCombinedName () <<
      " to stack" <<
      endl;
  }
#endif

  fPartGroupBlocksStack.top ()->
    appendElementToPartGroupBlock (
      fCurrentPartBlock);
}

void msr2LpsrTranslator::visitEnd (S_msrPart& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
  gIndenter--;

#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPart " <<
      elt->getPartCombinedName () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  string
    partInstrumentAbbreviation =
      fCurrentPartClone->
        getPartInstrumentAbbreviation ();
    
  // populate part instrument short name if empty and possible
  if (partInstrumentAbbreviation.size () == 0) {
    string
      partAbbreviation =
        elt->getPartAbbreviation ();
        
    fCurrentPartClone->
      setPartInstrumentAbbreviation (
        partAbbreviation);

    fCurrentPartClone->
      finalizePartClone (
        inputLineNumber);
  }

  if (fCurrentSkipGraceNotesGroup) {
    // add it ahead of the other voices in the part if needed
    fCurrentPartClone->
      addSkipGraceNotesGroupBeforeAheadOfVoicesClonesIfNeeded (
        fCurrentVoiceClone,
        fCurrentSkipGraceNotesGroup);

    // forget about this skip grace notes group
    fCurrentSkipGraceNotesGroup = nullptr;
  }
}

//________________________________________________________________________
/* JMI
void msr2LpsrTranslator::visitStart (S_msrStaffLinesNumber& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffLinesNumber" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create a staff lines number clone
  fCurrentStaffLinesNumberClone =
    elt->
      createStaffLinesNumberNewbornClone ();
}
*/

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStaffTuning& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffTuning" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
  
  // create a staff tuning clone
  fCurrentStaffTuningClone =
    elt->
      createStaffTuningNewbornClone ();
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStaffDetails& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffDetails" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentStaffTuningClone = nullptr;
}

void msr2LpsrTranslator::visitEnd (S_msrStaffDetails& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrStaffDetails" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the staff details to the current staff clone
  fCurrentStaffClone->
    appendStaffDetailsToStaff (
      elt);

        
/* JMI
  // add it to the staff clone
  fCurrentStaffClone->
    addStaffTuningToStaff (
      fCurrentStaffTuningClone);

  // create a staff tuning block
  S_lpsrNewStaffTuningBlock
    newStaffTuningBlock =
      lpsrNewStaffTuningBlock::create (
        fCurrentStaffTuningClone->getInputLineNumber (),
        fCurrentStaffTuningClone);

  // append it to the current staff block
  fCurrentStaffBlock->
    appendElementToStaffBlock (newStaffTuningBlock);
    */
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStaff& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaff \"" <<
      elt->getStaffName () << "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  gIndenter++;

  switch (elt->getStaffKind ()) {
    case msrStaff::kStaffRegular:
    case msrStaff::kStaffTablature:
    case msrStaff::kStaffDrum:
    case msrStaff::kStaffRythmic:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);
          
        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);
      
        // create a staff block
        fCurrentStaffBlock =
          lpsrStaffBlock::create (
            fCurrentStaffClone);
      
        string
          partName =
            fCurrentPartClone->getPartName (),
          partAbbreviation =
            fCurrentPartClone->getPartAbbreviation ();
      
        string staffBlockInstrumentName;
        string staffBlockShortInstrumentName;
      
        // don't set instrument name nor short instrument name // JMI
        // if the staff belongs to a piano part where they're already set
        if (! partName.size ()) {
          staffBlockInstrumentName = partName;
        }
        if (! partAbbreviation.size ()) {
          staffBlockShortInstrumentName = partAbbreviation;
        }
      
        if (staffBlockInstrumentName.size ()) {
          fCurrentStaffBlock->
            setStaffBlockInstrumentName (staffBlockInstrumentName);
        }
            
        if (staffBlockShortInstrumentName.size ()) {
          fCurrentStaffBlock->
            setStaffBlockShortInstrumentName (staffBlockShortInstrumentName);
        }
              
        // append the staff block to the current part block
        fCurrentPartBlock->
          appendStaffBlockToPartBlock (
            fCurrentStaffBlock);
      
        fOnGoingStaff = true;
      }
      break;
      
    case msrStaff::kStaffHarmony:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);
        
        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);
      
        fOnGoingStaff = true;
      }
      break;
      
    case msrStaff::kStaffFiguredBass:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);
        
        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);

        // register it as the part figured bass staff
        fCurrentPartClone->
          setPartFiguredBassStaff (fCurrentStaffClone);

        fOnGoingStaff = true;
      }
      break;
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrStaff& elt)
{
  gIndenter--;

#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting S_msrStaff \"" <<
      elt->getStaffName () << "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  switch (elt->getStaffKind ()) {
    case msrStaff::kStaffRegular:
    case msrStaff::kStaffDrum:
    case msrStaff::kStaffRythmic:
      {
        fOnGoingStaff = false;
      }
      break;
      
    case msrStaff::kStaffTablature:
      // JMI
      break;
      
    case msrStaff::kStaffHarmony:
      // JMI
      break;
      
    case msrStaff::kStaffFiguredBass:
      // JMI
      break;
  } // switch
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrVoice& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVoice \"" <<
      elt->asString () << "\"" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fCurrentVoiceOriginal = elt;
  
  gIndenter++;

  switch (elt->getVoiceKind ()) {
    
    case msrVoice::kRegularVoice:
      // create a voice clone
      fCurrentVoiceClone =
        elt->createVoiceNewbornClone (
          fCurrentStaffClone);
            
      // add it to the staff clone
      fCurrentStaffClone->
        registerVoiceInStaff (
          inputLineNumber,
          fCurrentVoiceClone);
    
      // append the voice clone to the LPSR score elements list
      fLpsrScore ->
        appendVoiceToScoreElements (fCurrentVoiceClone);
    
      // append a use of the voice to the current staff block
      fCurrentStaffBlock->
        appendVoiceUseToStaffBlock (
          fCurrentVoiceClone);
      break;
      
    case msrVoice::kHarmonyVoice:
      {
        /* JMI
        // create the harmony staff and voice if not yet done
        fCurrentPartClone->
          createPartHarmonyStaffAndVoiceIfNotYetDone (
            inputLineNumber);
          
        // fetch harmony voice
        fCurrentVoiceClone =
          fCurrentPartClone->
            getPartHarmonyVoice ();
*/

        // create a voice clone
        fCurrentVoiceClone =
          elt->createVoiceNewbornClone (
            fCurrentStaffClone);
              
        // add it to the staff clone
        fCurrentStaffClone->
          registerVoiceInStaff (
            inputLineNumber,
            fCurrentVoiceClone);
    
        if (
          elt->getMusicHasBeenInsertedInVoice () // superfluous test ??? JMI
          ) {          
          // append the voice clone to the LPSR score elements list
          fLpsrScore ->
            appendVoiceToScoreElements (
              fCurrentVoiceClone);
      
          // create a ChordNames context command
          string voiceName =
            elt->getVoiceName ();
  
          string partCombinedName =
            elt->fetchVoicePartUplink ()->
              getPartCombinedName ();
                          
#ifdef TRACE_OPTIONS
          if (gTraceOptions->fTraceHarmonies) {
            fLogOutputStream <<
              "Creating a ChordNames context for \"" << voiceName <<
              "\" in part " << partCombinedName <<
              endl;
          }
#endif
  
          S_lpsrChordNamesContext
            chordNamesContext =
              lpsrChordNamesContext::create (
                inputLineNumber,
                lpsrContext::kExistingContextYes,
                voiceName,
                elt->getRegularVoiceForHarmonyVoice ());
  
          // append it to the current part block
#ifdef TRACE_OPTIONS
          if (gTraceOptions->fTraceHarmonies) {
            fLogOutputStream <<
              "Appending the ChordNames context for \"" << voiceName <<
              "\" in part " << partCombinedName <<
              endl;
          }
#endif
  
          fCurrentPartBlock->
            appendChordNamesContextToPartBlock (
              inputLineNumber,
              chordNamesContext);
  
          fOnGoingHarmonyVoice = true;
        }
      }
      break;
      
    case msrVoice::kFiguredBassVoice:
      {
        /* JMI
        // create the figured bass staff and voice if not yet done
        fCurrentPartClone->
          createPartFiguredBassStaffAndVoiceIfNotYetDone (
            inputLineNumber);
          
        // fetch figured bass voice
        fCurrentVoiceClone =
          fCurrentPartClone->
            getPartFiguredBassVoice ();
*/

        // create a voice clone
        fCurrentVoiceClone =
          elt->createVoiceNewbornClone (
            fCurrentStaffClone);
              
        // add it to the staff clone
        fCurrentStaffClone->
          registerVoiceInStaff (
            inputLineNumber,
            fCurrentVoiceClone);
    
        // register it as the part figured bass voice
        fCurrentPartClone->
          setPartFiguredBassVoice (fCurrentVoiceClone);

        if (
          elt->getMusicHasBeenInsertedInVoice () // superfluous test ??? JMI
          ) {          
          // append the voice clone to the LPSR score elements list
          fLpsrScore ->
            appendVoiceToScoreElements (
              fCurrentVoiceClone);
      
          // create a FiguredBass context command
          string voiceName =
            elt->getVoiceName ();
  
          string partCombinedName =
            elt->fetchVoicePartUplink ()->
              getPartCombinedName ();
                          
#ifdef TRACE_OPTIONS
          if (gTraceOptions->fTraceHarmonies) {
            fLogOutputStream <<
              "Creating a FiguredBass context for \"" << voiceName <<
              "\" in part " << partCombinedName <<
              endl;
          }
#endif
  
          S_lpsrFiguredBassContext
            figuredBassContext =
              lpsrFiguredBassContext::create (
                inputLineNumber,
                lpsrContext::kExistingContextYes,
                voiceName,
                elt-> getVoiceStaffUplink ());
  
          // append it to the current part block
#ifdef TRACE_OPTIONS
          if (gTraceOptions->fTraceHarmonies) {
            fLogOutputStream <<
              "Appending the FiguredBass context for \"" << voiceName <<
              "\" in part " << partCombinedName <<
              endl;
          }
#endif
  
          fCurrentPartBlock->
            appendFiguredBassContextToPartBlock (
              figuredBassContext);
  
          fOnGoingFiguredBassVoice = true;
        }
      }
      break;
  } // switch

  // clear the voice notes map
  fVoiceNotesMap.clear ();

  fFirstNoteCloneInVoice = nullptr;
}

void msr2LpsrTranslator::visitEnd (S_msrVoice& elt)
{
  gIndenter--;

#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrVoice \"" <<
      elt->getVoiceName () << "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  switch (elt->getVoiceKind ()) {
    case msrVoice::kRegularVoice:
      // JMI
      break;
      
    case msrVoice::kHarmonyVoice:
      fOnGoingHarmonyVoice = false;
      break;
      
    case msrVoice::kFiguredBassVoice:
      fOnGoingFiguredBassVoice = false;
      break;
  } // switch
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrVoiceStaffChange& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVoiceStaffChange '" <<
      elt->asString () << "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a voice staff change clone
  S_msrVoiceStaffChange
    voiceStaffChangeClone =
      elt->
        createStaffChangeNewbornClone ();

  // append it to the current voice clone
  fCurrentVoiceClone->
    appendVoiceStaffChangeToVoice (
      voiceStaffChangeClone);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSegment& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSegment '" <<
      elt->getSegmentAbsoluteNumber () << "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a clone of the segment
  fCurrentSegmentClone =
    elt->createSegmentNewbornClone (
      fCurrentVoiceClone);

  // set it as the new voice last segment
  fCurrentVoiceClone->
    setVoiceLastSegmentInVoiceClone (
      fCurrentSegmentClone);
}

void msr2LpsrTranslator::visitEnd (S_msrSegment& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSegment '" <<
      elt->getSegmentAbsoluteNumber () << "'" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif
    
  fCurrentVoiceClone->
    handleSegmentCloneEndInVoiceClone (
      inputLineNumber,
      fCurrentSegmentClone);
      
  // forget current segment clone
  fCurrentSegmentClone = nullptr;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrHarmony& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrHarmony '" <<
      elt->asString () <<
      ", fOnGoingNote = " << booleanAsString (fOnGoingNote) <<
      ", fOnGoingChord = " << booleanAsString (fOnGoingChord) <<
      ", fOnGoingHarmonyVoice = " << booleanAsString (fOnGoingHarmonyVoice) <<
      "', line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a harmony new born clone
  fCurrentHarmonyClone =
    elt->createHarmonyNewbornClone (
      fCurrentVoiceClone);
      
  if (fOnGoingNote) {
    // register the harmony in the current non-grace note clone
    fCurrentNonGraceNoteClone->
      setNoteHarmony (fCurrentHarmonyClone);

  // don't append the harmony to the part harmony,
  // this has been done in pass2b
  }
  
  else if (fOnGoingChord) {
    // register the harmony in the current chord clone
    fCurrentChordClone->
      setChordHarmony (fCurrentHarmonyClone); // JMI
  }

  else if (fOnGoingHarmonyVoice) {
    fCurrentVoiceClone->
      appendHarmonyToVoiceClone (
        fCurrentHarmonyClone);
  }
}

void msr2LpsrTranslator::visitStart (S_msrHarmonyDegree& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_msrHarmonyDegree '" <<
      elt->asString () <<
      ", fOnGoingNote = " << booleanAsString (fOnGoingNote) <<
      ", fOnGoingChord = " << booleanAsString (fOnGoingChord) <<
      ", fOnGoingHarmonyVoice = " << booleanAsString (fOnGoingHarmonyVoice) <<
      "', line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the harmony degree to the current harmony clone
  fCurrentHarmonyClone->
    appendHarmonyDegreeToHarmony (
      elt);
}

void msr2LpsrTranslator::visitEnd (S_msrHarmony& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrHarmony '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrFrame& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFrame '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    // register the frame in the current non-grace note clone
    fCurrentNonGraceNoteClone->
      setNoteFrame (elt);
  }
}  

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrFiguredBass& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFiguredBass '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a deep copy of the figured bass
  fCurrentFiguredBass =
    elt->
      createFiguredBassDeepCopy (
        fCurrentPartClone);
  
  if (fOnGoingNote) {
    // register the figured bass in the current non-grace note clone
    fCurrentNonGraceNoteClone->
      setNoteFiguredBass (fCurrentFiguredBass);

  // don't append the figured bass to the part figured bass,
  // this will be done below
  }
  
  else if (fOnGoingChord) {
    // register the figured bass in the current chord clone
    fCurrentChordClone->
      setChordFiguredBass (fCurrentFiguredBass); // JMI
  }
  
  else if (fOnGoingFiguredBassVoice) { // JMI
    // register the figured bass in the part clone figured bass
    fCurrentPartClone->
      appendFiguredBassToPartClone (
        fCurrentVoiceClone,
        fCurrentFiguredBass);
  }
}

void msr2LpsrTranslator::visitStart (S_msrFigure& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFigure '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the figure to the current figured bass
  fCurrentFiguredBass->
    appendFiguredFigureToFiguredBass (
      elt);
}

void msr2LpsrTranslator::visitEnd (S_msrFiguredBass& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrFiguredBass '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentFiguredBass = nullptr;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMeasure& elt)
{    
#ifdef TRACE_OPTIONS
  int
    inputLineNumber =
      elt->getInputLineNumber ();
#endif

  string
    measureNumber =
      elt->getMeasureNumber ();

#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasure '" <<
      measureNumber <<
      "'" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceMeasures) {
    fLogOutputStream <<
      endl <<
      "<!--=== measure '" << measureNumber <<
      "', voice \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      ", line " << inputLineNumber << " ===-->" <<
      endl;
  }
#endif
  
  // create a clone of the measure
  fCurrentMeasureClone =
    elt->
      createMeasureNewbornClone (
        fCurrentSegmentClone);
      
  // append it to the current segment clone
  fCurrentSegmentClone->
    appendMeasureToSegment (
      fCurrentMeasureClone);
      
// JMI utile???
  fCurrentPartClone->
    setPartCurrentMeasureNumber (
      measureNumber);

  // should the last bar check's measure be set?
  if (fLastBarCheck) {
    fLastBarCheck->
      setNextBarNumber (
        measureNumber);
      
    fLastBarCheck = nullptr;
  }
}

void msr2LpsrTranslator::visitEnd (S_msrMeasure& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
  string
    measureNumber =
      elt->getMeasureNumber ();

#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasure '" <<
      measureNumber <<
      "'" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fCurrentMeasureClone->
    finalizeMeasureClone (
      inputLineNumber,
      elt); // original measure

  bool doCreateABarCheck = false; // JMI

  switch (elt->getMeasureKind ()) {
    
    case msrMeasure::kMeasureKindUnknown:
      {
        stringstream s;

        s <<
          "measure '" << measureNumber <<
          "' in voice \"" <<
          elt->
            fetchMeasureVoiceUplink ()->
              getVoiceName () <<
          "\" is of unknown kind";

      // JMI  msrInternalError (
        msrInternalWarning (
          gGeneralOptions->fInputSourceName,
          inputLineNumber,
  //        __FILE__, __LINE__,
          s.str ());
      }
      break;
      
    case msrMeasure::kMeasureKindRegular:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kMeasureKindAnacrusis:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kMeasureKindIncompleteIrrelevantToAnyRepeat:
    case msrMeasure::kMeasureKindIncompleteLastInRepeatCommonPart:
    case msrMeasure::kMeasureKindIncompleteLastInRepeatHookedEnding:
    case msrMeasure::kMeasureKindIncompleteLastInRepeatHooklessEnding:
      doCreateABarCheck = true;
      break;

    case msrMeasure::kMeasureKindOvercomplete:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kMeasureKindCadenza:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kMeasureKindEmpty:
      // JMI
      break;
  } // switch

  if (doCreateABarCheck) {
    // create a bar check without next bar number,
    // it will be set upon visitStart (S_msrMeasure&)
    // for the next measure
    fLastBarCheck =
      msrBarCheck::create (
        inputLineNumber);

    // append it to the current voice clone
    fCurrentVoiceClone->
      appendBarCheckToVoice (fLastBarCheck);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStanza& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStanza \"" <<
      elt->getStanzaName () <<
      "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  gIndenter++;

//  if (elt->getStanzaTextPresent ()) { // JMI
    fCurrentStanzaClone =
      elt->createStanzaNewbornClone (
        fCurrentVoiceClone);
    
    // append the stanza clone to the LPSR score elements list
    fLpsrScore ->
      appendStanzaToScoreElements (
        fCurrentStanzaClone);
  
    // append a use of the stanza to the current staff block
    fCurrentStaffBlock ->
      appendLyricsUseToStaffBlock (
        fCurrentStanzaClone);
//  }
//  else
  //  fCurrentStanzaClone = 0; // JMI

  fOnGoingStanza = true;
}

void msr2LpsrTranslator::visitEnd (S_msrStanza& elt)
{
  gIndenter--;
  
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrStanza \"" <<
      elt->getStanzaName () <<
      "\"" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // forget about this stanza
  fCurrentStanzaClone = nullptr;
  
  fOnGoingStanza = false;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSyllable& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSyllable" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // create the syllable clone
  fCurrentSyllableClone =
    elt->createSyllableNewbornClone (
      fCurrentPartClone);

  // add it to the current stanza clone or current note clone
  if (fOnGoingStanza) { // fCurrentStanzaClone JM
    // visiting a syllable as a stanza member
    fCurrentStanzaClone->
      appendSyllableToStanza (
        fCurrentSyllableClone);
  }
  
  else if (fOnGoingNote) { // JMI
    // visiting a syllable as attached to the current non-grace note
    fCurrentSyllableClone->
      appendSyllableToNoteAndSetItsNoteUplink (
        fCurrentNonGraceNoteClone);

    if (gLpsrOptions->fAddWordsFromTheLyrics) {
      // get the syllable texts list
      const list<string>&
        syllableTextsList =
          elt->getSyllableTextsList ();

      if (syllableTextsList.size ()) {
        // build a single words value from the texts list
        // JMI create an msrWords instance for each???  
        string wordsValue =
          elt->syllableTextsListAsString();
  
        // create the words
#ifdef TRACE_OPTIONS
        if (gTraceOptions->fTraceLyrics || gTraceOptions->fTraceWords) {
          fLogOutputStream <<
            "Changing lyrics '" <<
            wordsValue <<
            "' into words for note '" <<
            fCurrentNonGraceNoteClone->asShortString () <<
            "'" <<
      // JMI      fCurrentSyllableClone->asString () <<
            endl;
        }
#endif

        S_msrWords
          words =
            msrWords::create (
              inputLineNumber,
              kPlacementNone,                // default value
              wordsValue,
              kJustifyNone,                  // default value
              kHorizontalAlignmentNone,      // default value
              kVerticalAlignmentNone,        // default value
              kFontStyleNone,                // default value
              msrFontSize::create (
                msrFontSize::kFontSizeNone), // default value
              kFontWeightNone,               // default value
              kXMLLangIt);                   // default value
  
        // append it to the current non-grace note
#ifdef TRACE_OPTIONS
        if (gTraceOptions->fTraceLyrics || gTraceOptions->fTraceWords) {
          fLogOutputStream <<
            "Appending words '" <<
            words->asShortString () <<
            "' to note '" <<
            fCurrentNonGraceNoteClone->asShortString () <<
            "'" <<
            endl;
        }
#endif
        fCurrentNonGraceNoteClone->
          appendWordsToNote (
            words);
      }
    }
  }
    
  // a syllable ends the sysllable extend range if any
  if (fOnGoingSyllableExtend) {
    /* JMI ???
    // create melisma end command
    S_lpsrMelismaCommand
      melismaCommand =
        lpsrMelismaCommand::create (
          inputLineNumber,
          lpsrMelismaCommand::kMelismaEnd);

    // append it to current voice clone
    fCurrentVoiceClone->
      appendOtherElementToVoice (melismaCommand);
*/

    fOnGoingSyllableExtend = false;
  }
}

void msr2LpsrTranslator::visitEnd (S_msrSyllable& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSyllable" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrClef& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrClef" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendClefToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrClef& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrClef" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrKey& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrKey" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendKeyToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrKey& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrKey" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTime& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTime" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append time to voice clone
  fCurrentVoiceClone->
    appendTimeToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrTime& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTime" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTranspose& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTranspose" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append transpose to voice clone
  fCurrentVoiceClone->
    appendTransposeToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrTranspose& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTranspose" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPartNameDisplay& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPartNameDisplay" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append part name display to voice clone
  fCurrentVoiceClone->
    appendPartNameDisplayToVoice (elt);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPartAbbreviationDisplay& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPartAbbreviationDisplay" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append part abbreviation display to voice clone
  fCurrentVoiceClone->
    appendPartAbbreviationDisplayToVoice (elt);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTempo& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTempo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  switch (elt->getTempoKind ()) {
    case msrTempo::k_NoTempoKind:
      break;

    case msrTempo::kTempoBeatUnitsPerMinute:
      break;

    case msrTempo::kTempoBeatUnitsEquivalence:
      break;

    case msrTempo::kTempoNotesRelationShip:
      fLpsrScore->
        // this score needs the 'tongue' Scheme function
        setTempoRelationshipSchemeFunctionIsNeeded ();
      break;
  } // switch

  fCurrentVoiceClone->
    appendTempoToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrTempo& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTempo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRehearsal& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRehearsal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendRehearsalToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrRehearsal& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRehearsal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrArticulation& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrArticulation" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendArticulationToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendArticulationToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrArticulation& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrArticulation" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrFermata& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFermata" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // a fermata is an articulation
  
  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendArticulationToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendArticulationToChord (elt);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrArpeggiato& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrArpeggiato" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // an arpeggiato is an articulation
  
  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendArticulationToNote (elt); // addArpeggiatoToNote ??? JMI
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendArticulationToChord (elt);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrNonArpeggiato& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrNonArpeggiato" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // an nonArpeggiato is an articulation
  
  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendArticulationToNote (elt); // addArpeggiatoToNote ??? JMI
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendArticulationToChord (elt);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTechnical& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnical" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendTechnicalToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTechnicalToChord (elt);
  }

  // doest the score need the 'tongue' function?
  switch (elt->getTechnicalKind ()) {
    case msrTechnical::kArrow:
      break;
    case msrTechnical::kDoubleTongue:
      fLpsrScore->
        // this score needs the 'tongue' Scheme function
        setTongueSchemeFunctionIsNeeded ();
      break;
    case msrTechnical::kDownBow:
      break;
    case msrTechnical::kFingernails:
      break;
    case msrTechnical::kHarmonic:
      break;
    case msrTechnical::kHeel:
      break;
    case msrTechnical::kHole:
      break;
    case msrTechnical::kOpenString:
      break;
    case msrTechnical::kSnapPizzicato:
      break;
    case msrTechnical::kStopped:
      break;
    case msrTechnical::kTap:
      break;
    case msrTechnical::kThumbPosition:
      break;
    case msrTechnical::kToe:
      break;
    case msrTechnical::kTripleTongue:
      fLpsrScore->
        // this score needs the 'tongue' Scheme function
        setTongueSchemeFunctionIsNeeded ();
      break;
    case msrTechnical::kUpBow:
      break;
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrTechnical& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnical" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTechnicalWithInteger& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnicalWithInteger" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendTechnicalWithIntegerToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTechnicalWithIntegerToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrTechnicalWithInteger& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnicalWithInteger" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTechnicalWithFloat& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnicalWithFloat" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendTechnicalWithFloatToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTechnicalWithFloatToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrTechnicalWithFloat& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnicalWithFloat" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTechnicalWithString& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnicalWithString" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendTechnicalWithStringToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendTechnicalWithStringToChord (elt);
  }
  
  switch (elt->getTechnicalWithStringKind ()) {
    case msrTechnicalWithString::kHammerOn:
    case msrTechnicalWithString::kPullOff:
      // this score needs the 'after' Scheme function
      fLpsrScore->
        setAfterSchemeFunctionIsNeeded ();     
      break;
    default:
      ;
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrTechnicalWithString& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnicalWithString" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrOrnament& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOrnament" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendOrnamentToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendOrnamentToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrOrnament& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOrnament" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSpanner& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSpanner" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  switch (elt->getSpannerTypeKind ()) {
    case kSpannerTypeStart:
      break;
    case kSpannerTypeStop:
      break;
    case kSpannerTypeContinue:
      break;
    case k_NoSpannerType:
      break;
  } // switch

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendSpannerToNote (elt);
  }
  
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendSpannerToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrSpanner& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSpanner" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrGlissando& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrGlissando" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendGlissandoToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendGlissandoToChord (elt);
  }

  if (elt->getGlissandoTextValue ().size ()) {
    fLpsrScore->
      // this score needs the 'glissandoWithText' Scheme function
      addGlissandoWithTextSchemeFunctionsToScore ();
  }
}

void msr2LpsrTranslator::visitEnd (S_msrGlissando& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrGlissando" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSlide& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSlide" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendSlideToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendSlideToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrSlide& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSlide" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSingleTremolo& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      setNoteSingleTremolo (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      setChordSingleTremolo (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrSingleTremolo& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrDoubleTremolo& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrDoubleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create a double tremolo clone from the two elements
  fCurrentDoubleTremoloClone = elt; // JMI FIX THAT
/* JMI
    elt->createDoubleTremoloNewbornClone (
      elt->getDoubleTremoloFirstElement ()->
        createNewBornClone (),
      elt->getDoubleTremoloSecondElement ()
        createNewBornClone ());
        */
  
  fOnGoingDoubleTremolo = true;
}

void msr2LpsrTranslator::visitEnd (S_msrDoubleTremolo& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the current double tremolo clone to the current voice clone
  fCurrentVoiceClone->
    appendDoubleTremoloToVoice (
      fCurrentDoubleTremoloClone);

  // forget about it
  fCurrentDoubleTremoloClone = nullptr;
  
  fOnGoingDoubleTremolo = false;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrDynamics& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendDynamicsToNote (elt);

    // is this a non LilyPond native dynamics?
    bool knownToLilyPondNatively = true;
    
    switch (elt->getDynamicsKind ()) {
      case msrDynamics::kFFFFF:
      case msrDynamics::kFFFFFF:
      case msrDynamics::kPPPPP:
      case msrDynamics::kPPPPPP:
      case msrDynamics::kRF:
      case msrDynamics::kSFPP:
      case msrDynamics::kSFFZ:
      case msrDynamics::k_NoDynamics:
        knownToLilyPondNatively = false;
          
      default:
        ;
    } // switch
  
    if (! knownToLilyPondNatively) {
      // this score needs the 'dynamics' Scheme function
      fLpsrScore->
        setDynamicsSchemeFunctionIsNeeded ();   
    }
  }
  
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendDynamicsToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrDynamics& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrOtherDynamics& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOtherDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendOtherDynamicsToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendOtherDynamicsToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrOtherDynamics& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOtherDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrWords& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendWordsToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendWordsToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrWords& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSlur& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSlur" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  /*
    Only the  first note of the chord should get the slur notation.
    Some applications print out the slur for all notes,
    i.e. a stop and a start in sequqnce:
    these should be ignored
  */

  if (fOnGoingNote) {
    // don't add slurs to chord member notes except the first one
    switch (fCurrentNonGraceNoteClone->getNoteKind ()) {
      case msrNote::kChordMemberNote:
        if (fCurrentNonGraceNoteClone->getNoteIsAChordsFirstMemberNote ()) {
          fCurrentNonGraceNoteClone->
            appendSlurToNote (elt);
        }
        break;
        
      default:
        fCurrentNonGraceNoteClone->
          appendSlurToNote (elt);
    } // switch
  }
  
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendSlurToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrSlur& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSlur" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrLigature& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrLigature" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendLigatureToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendLigatureToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrLigature& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrLigature" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSlash& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSlash" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendSlashToNote (elt);
  }
  
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendSlashToChord (elt);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrWedge& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrWedge" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendWedgeToNote (elt);
  }
  
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendWedgeToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrWedge& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrWedge" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrGraceNotesGroup& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber () ;
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrGraceNotesGroup" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  bool doCreateAGraceNoteClone = true; // JMI

  if (doCreateAGraceNoteClone) {
    // create a clone of this graceNotesGroup
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTraceNotes || gTraceOptions->fTraceGraceNotes) {
      fLogOutputStream <<
        "Creating a clone of grace notes group '" << 
        elt->asShortString () <<
        "' and attaching it to clone note '" <<
        fCurrentNonGraceNoteClone->asShortString () <<
        "'" << 
        endl;
      }
#endif

    fCurrentGraceNotesGroupClone =
      elt->
        createGraceNotesGroupNewbornClone (
          fCurrentVoiceClone);

    // attach it to the current note clone
    // if (fOnGoingNote) { JMI
   // { // JMI
    
    switch (elt->getGraceNotesGroupKind ()) {
      case msrGraceNotesGroup::kGraceNotesGroupBefore:
        fCurrentNonGraceNoteClone->
          setNoteGraceNotesGroupBefore (
            fCurrentGraceNotesGroupClone);
        break;
      case msrGraceNotesGroup::kGraceNotesGroupAfter:
        fCurrentNonGraceNoteClone->
          setNoteGraceNotesGroupAfter (
            fCurrentGraceNotesGroupClone);
        break;
    } // switch
  //  }
  }

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceNotes || gTraceOptions->fTraceGraceNotes) {
    fLogOutputStream <<
      "+++++++++++++++++++++++++ 1" <<
      endl <<
      "fCurrentNonGraceNoteClone:";
  
    if (fCurrentNonGraceNoteClone) {
      fLogOutputStream <<
        fCurrentNonGraceNoteClone;
    }
    else {
      fLogOutputStream <<
        "nullptr";
    }
    fLogOutputStream <<
      endl;
  }
#endif

  // get the note this grace notes group is attached to
  S_msrNote
    noteNotesGroupIsAttachedTo =
      elt->
        getGraceNotesGroupNoteUplink ();

  if (! noteNotesGroupIsAttachedTo) {
    stringstream s;

    s <<
      "grace notes group '" << elt->asShortString () <<
      "' has an empty note uplink";

    msrInternalError (
      gGeneralOptions->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      s.str ());
  }
  
  fOnGoingGraceNotesGroup = true;

  // is noteNotesGroupIsAttachedTo the first one in its voice?
#ifdef TRACE_OPTIONS
  if (
    gTraceOptions->fTraceGraceNotes
      ||
    gTraceOptions->fTraceNotes
      ||
    gTraceOptions->fTraceVoices
  ) {
    fLogOutputStream <<
      "The noteNotesGroupIsAttachedTo voice clone PEOJIOFEIOJEF '" <<
      fCurrentVoiceClone->getVoiceName () <<
      "' is '";

    if (noteNotesGroupIsAttachedTo) {
      fLogOutputStream <<
        noteNotesGroupIsAttachedTo->asShortString ();
    }
    else {
      fLogOutputStream <<
        "none";
    }
    fLogOutputStream <<
       "'" <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
  if (
    gTraceOptions->fTraceGraceNotes
      ||
    gTraceOptions->fTraceNotes
      ||
    gTraceOptions->fTraceVoices
  ) {
    fLogOutputStream <<
      "The first note of voice clone KLJWLPOEF '" <<
      fCurrentVoiceClone->getVoiceName () <<
      "' is '";

    if (fFirstNoteCloneInVoice) {
      fLogOutputStream <<
        fFirstNoteCloneInVoice->asShortString ();
    }
    else {
      fLogOutputStream <<
        "none";
    }
    fLogOutputStream <<
       "'" <<
      endl;
  }
#endif

  // fetch the original voice first non grace note
  S_msrNote
    originalVoiceFirstNonGraceNote =
      fCurrentVoiceOriginal->
        fetchVoiceFirstNonGraceNote ();

  if (originalVoiceFirstNonGraceNote) {
    if (noteNotesGroupIsAttachedTo == originalVoiceFirstNonGraceNote) {
      // bug 34 in LilyPond should be worked around by creating
      // skip grace notes in the other voices of the part
    
      // create the skip grace notes group
  #ifdef TRACE_OPTIONS
        if (
            gTraceOptions->fTraceGraceNotes
              ||
            gTraceOptions->fTraceNotes
              ||
            gTraceOptions->fTraceVoices
        ) {
          fLogOutputStream <<
            "Creating a skip clone of grace notes group '" <<
            elt->asShortString () <<
            "' to work around LilyPond issue 34" <<
            endl;
        }
  #endif
  
      fCurrentSkipGraceNotesGroup =
        elt->
          createSkipGraceNotesGroupClone (
            fCurrentVoiceClone);
    }
  }

  // addSkipGraceNotesGroupBeforeAheadOfVoicesClonesIfNeeded() will
  // append the same skip grace notes to the ofhter voices if needed
  // in visitEnd (S_msrPart&)
}

    /* JMI
  if (fFirstNoteCloneInVoice) {
    // there is at least a note before these grace notes in the voice
    
    if (
      fCurrentNonGraceNoteClone->getNoteTrillOrnament ()
        &&
      fCurrentNonGraceNoteClone->getNoteIsFollowedByGraceNotesGroup ()) {
      // fPendingAfterGraceNotesGroup already contains
      // the afterGraceNotesGroup to use
      
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceGraceNotesGroup) {
        fLogOutputStream <<
          "Optimising grace notes '" << 
          elt->asShortString () <<
          "' into after grace notes" <<
          endl;
      }
#endif

      // attach the current after grace notes clone to the current note clone
      if (fOnGoingNote) { // JMI
        fCurrentNonGraceNoteClone->
          setNoteAfterGraceNotesGroup (
            fPendingAfterGraceNotesGroup);
      }

      doCreateAGraceNoteClone = false;
    }
  }

  if (doCreateAGraceNoteClone) {
    // are these grace notes the last element in a measure?
    if (elt->getGraceNotesGroupIsFollowedByNotes ()) {
      // yes, this is a regular grace notes

      // create a clone of this graceNotesGroup
      fCurrentGraceNotesGroupClone =
        elt->
          createGraceNotesGroupNewbornClone (
            fCurrentVoiceClone);

      // attach it to the current note clone
      if (fOnGoingNote) { // JMI
        fCurrentNonGraceNoteClone->
          setNoteGraceNotesGroup (
            fCurrentGraceNotesGroupClone);
      }

     // JMI XXL find good criterion for this
  
      // these grace notes are at the beginning of a segment JMI
  //    doCreateAGraceNoteClone = true; // JMI
  
      // bug 34 in LilyPond should be worked aroud by creating
      // skip grace notes in the other voices of the part
  
      // create skip graceNotesGroup clone
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceGraceNotesGroup) {
        fLogOutputStream <<
          "Creating a skip clone of grace notes '" <<
          elt->asShortString () <<
          "' to work around LilyPond issue 34" <<
          endl;
      }
#endif
    
      S_msrGraceNotesGroup
        skipGraceNotesGroup =
          elt->
            createSkipGraceNotesGroupClone (
              fCurrentVoiceClone);
  
      // prepend it to the other voices in the part
      fCurrentPartClone->
        prependSkipGraceNotesGroupToVoicesClones (
          fCurrentVoiceClone,
          skipGraceNotesGroup);
    }

    else {
      // no, we should build an msrAfterGraceNotesGroup from this
      // and the last element in the current voice clone

      // fetch the voice last element
      fCurrentAfterGraceNotesGroupElement =
        fCurrentVoiceClone->
          fetchVoiceLastElement (inputLineNumber);

      // create the after grace notes
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceGraceNotesGroup) {
        fLogOutputStream <<
          "Converting grace notes '" <<
          elt->asShortString () <<
          "' into after grace notes attached to:"<<
          endl;

        gIndenter++;
        
        fCurrentAfterGraceNotesGroupElement->
          print (fLogOutputStream);

        gIndenter--;
      }
#endif

      fPendingAfterGraceNotesGroup =
        msrAfterGraceNotesGroup::create (
          inputLineNumber,
            fCurrentAfterGraceNotesGroupElement,
            elt->getGraceNotesGroupIsSlashed (),
            fCurrentVoiceClone);

      // append it to the current note clone
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceGraceNotesGroup) {
        fLogOutputStream <<
          "Appending the after grace notes to current note clone" <<
          endl;
      }
#endif
      
      if (fOnGoingNote) { // JMI
        fCurrentNonGraceNoteClone->
          setNoteAfterGraceNotesGroup (
            fPendingAfterGraceNotesGroup);
      }
    }
  }
*/


void msr2LpsrTranslator::visitEnd (S_msrGraceNotesGroup& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrGraceNotesGroup" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTraceNotes || gTraceOptions->fTraceGraceNotes) {
    fLogOutputStream <<
      "+++++++++++++++++++++++++ 2" <<
      endl <<
      "fCurrentNonGraceNoteClone:";
  
    if (fCurrentNonGraceNoteClone) {
      fLogOutputStream <<
        fCurrentNonGraceNoteClone;
    }
    else {
      fLogOutputStream <<
        "nullptr";
    }
    fLogOutputStream <<
      endl;
  }
#endif

  // forget about these grace notes
  fCurrentGraceNotesGroupClone = nullptr;

  fOnGoingGraceNotesGroup = false;
  
/* JMI
  if (fPendingAfterGraceNotesGroup) {
    // remove the current afterGraceNotesGroup note clone
    // from the current voice clone
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTraceGraceNotesGroup) {
      fLogOutputStream <<
        "Removing the after grace notes element from the current voice clone" <<
        endl;
    }
#endif

    fCurrentVoiceClone->
      removeElementFromVoice (
        inputLineNumber,
        fCurrentAfterGraceNotesGroupElement);

    // forget about the current after grace notes element
    fCurrentAfterGraceNotesGroupElement = nullptr;
  
    // forget about these after the pending grace notes
    fPendingAfterGraceNotesGroup = nullptr;
  }
  */
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrNote& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrNote '" <<
      elt->asString () <<
      "'" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
    
  // create the note clone
  S_msrNote
    noteClone =
      elt->createNoteNewbornClone (
        fCurrentPartClone);

  // register clone in this tranlastors' voice notes map
  fVoiceNotesMap [elt] = noteClone; // JMI XXL
  
  // don't register grace notes as the current note clone,
  // but as the current grace note clone instead
/* JMI
#ifdef TRACE_OPTIONS
        if (gTraceOptions->fTraceNotes || gTraceOptions->fTraceVoices) {
          fLogOutputStream <<
            "The first note of voice clone GFFF '" <<
            fCurrentVoiceClone->getVoiceName () <<
            "' is '";

          if (fFirstNoteCloneInVoice) {
            fLogOutputStream <<
              fFirstNoteCloneInVoice->asShortString ();
          }
          else {
            fLogOutputStream <<
              "none";
          }
          fLogOutputStream <<
             "'" <<
            endl;
        }
#endif
*/

  switch (elt->getNoteKind ()) {
    
    case msrNote::kGraceNote:
    case msrNote::kGraceChordMemberNote:
    case msrNote::kGraceTupletMemberNote:
      fCurrentGraceNoteClone = noteClone;
      break;
      
    default:
      fCurrentNonGraceNoteClone = noteClone;

      if (! fFirstNoteCloneInVoice) {
        fFirstNoteCloneInVoice =
          fCurrentNonGraceNoteClone;

#ifdef TRACE_OPTIONS
        if (gTraceOptions->fTraceNotes || gTraceOptions->fTraceVoices) {
          fLogOutputStream <<
            "The first note of voice clone RJIRWR '" <<
            fCurrentVoiceClone->getVoiceName () <<
            "' is '" <<
            fFirstNoteCloneInVoice->asShortString () <<
             "'" <<
            endl;
        }
#endif
      }

      fOnGoingNote = true;
  } // switch

/* JMI
  // can we optimize graceNotesGroup into afterGraceNotesGroup?
  if (
    elt->getNoteIsFollowedByGraceNotesGroup ()
      &&
    elt->getNoteTrillOrnament ()) {
    // yes, create the after grace notes
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTraceNotesGroup) {
      fLogOutputStream <<
        "Optimizing grace notes on trilled note '" <<
        elt->asShortString () <<
        "' as after grace notes " <<
        ", line " << inputLineNumber <<
        endl;
    }
#endif

    fPendingAfterGraceNotesGroup =
      msrAfterGraceNotesGroup::create (
        inputLineNumber,
        fCurrentNonGraceNoteClone,
        false, // aftergracenoteIsSlashed, may be updated later
        fCurrentVoiceClone);

    // register current afterGraceNotesGroup element
    fCurrentAfterGraceNotesGroupElement =
      fCurrentNonGraceNoteClone;
  }
*/
}

void msr2LpsrTranslator::visitEnd (S_msrNote& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrNote " <<
      elt->asString () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceNotesDetails) {
    fLogOutputStream <<
      "FAA fCurrentNonGraceNoteClone = " <<
      endl;
    if (fCurrentNonGraceNoteClone) {
      fLogOutputStream <<
        fCurrentNonGraceNoteClone;
    }
    else {
      fLogOutputStream <<
        "nullptr" <<
        endl;
    }
    
    fLogOutputStream <<
      "FAA fCurrentGraceNoteClone = " <<
      endl;
    if (fCurrentGraceNoteClone) {
      fLogOutputStream <<
        fCurrentGraceNoteClone;
    }
    else {
      fLogOutputStream <<
        "nullptr" <<
        endl;
    }
  }
#endif

  switch (elt->getNoteKind ()) {
    
    case msrNote::k_NoNoteKind:
      break;
      
    case msrNote::kRestNote:
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending rest note clone '" <<
          fCurrentNonGraceNoteClone->asShortString () << "' to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
#endif
          
      fCurrentVoiceClone->
        appendNoteToVoiceClone (
          fCurrentNonGraceNoteClone);
      break;
      
    case msrNote::kSkipNote: // JMI
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending skip note clone '" <<
          fCurrentNonGraceNoteClone->asShortString () << "' to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
#endif
          
      fCurrentVoiceClone->
        appendNoteToVoiceClone (
          fCurrentNonGraceNoteClone);
      break;
      
    case msrNote::kUnpitchedNote:
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending unpitched note clone '" <<
          fCurrentNonGraceNoteClone->asShortString () << "' to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
#endif
          
      fCurrentVoiceClone->
        appendNoteToVoiceClone (
          fCurrentNonGraceNoteClone);
      break;
      
    case msrNote::kStandaloneNote:
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending standalone note clone '" <<
          fCurrentNonGraceNoteClone->asShortString () << "' to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
#endif
          
      fCurrentVoiceClone->
        appendNoteToVoiceClone (
          fCurrentNonGraceNoteClone);
      break;
      
    case msrNote::kDoubleTremoloMemberNote:
      if (fOnGoingDoubleTremolo) {
        
        if (fCurrentNonGraceNoteClone->getNoteIsFirstNoteInADoubleTremolo ()) {
#ifdef TRACE_OPTIONS
          if (gTraceOptions->fTraceNotes) {
            fLogOutputStream <<
              "Setting note '" <<
              fCurrentNonGraceNoteClone->asString () <<
              "', line " << fCurrentNonGraceNoteClone->getInputLineNumber () <<
              ", as double tremolo first element" <<
              " in voice \"" <<
              fCurrentVoiceClone->getVoiceName () <<
              "\"" <<
              endl;
          }
#endif
              
          fCurrentDoubleTremoloClone->
            setDoubleTremoloNoteFirstElement (
              fCurrentNonGraceNoteClone);
        }
        
        else if (fCurrentNonGraceNoteClone->getNoteIsSecondNoteInADoubleTremolo ()) {
#ifdef TRACE_OPTIONS
          if (gTraceOptions->fTraceNotes) {
            fLogOutputStream <<
              "Setting note '" <<
              fCurrentNonGraceNoteClone->asString () <<
              "', line " << fCurrentNonGraceNoteClone->getInputLineNumber () <<
              ", as double tremolo second element" <<
              " in voice \"" <<
              fCurrentVoiceClone->getVoiceName () <<
              "\"" <<
              endl;
          }
#endif
              
          fCurrentDoubleTremoloClone->
            setDoubleTremoloNoteSecondElement (
              fCurrentNonGraceNoteClone);
        }
        
        else {
          stringstream s;

          s <<
            "note '" << fCurrentNonGraceNoteClone->asShortString () <<
            "' belongs to a double tremolo, but is not marked as such";

          msrInternalError (
            gGeneralOptions->fInputSourceName,
            inputLineNumber,
            __FILE__, __LINE__,
            s.str ());
        }
      }

      else {
        stringstream s;

        s <<
          "double tremolo note '" << fCurrentNonGraceNoteClone->asShortString () <<
          "' found outside of a double tremolo";

        msrInternalError (
          gGeneralOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
      }
      break;
      
    case msrNote::kGraceNote:
    /* JMI
      fLogOutputStream <<
        "fOnGoingGraceNotesGroup = " <<
        booleanAsString (
          fOnGoingGraceNotesGroup) <<
        endl;
        */
        
      if (! fOnGoingGraceNotesGroup) {
        stringstream s;

        s <<
          "grace note '" << fCurrentNonGraceNoteClone->asShortString () <<
          "' found outside of grace notes";

        msrInternalError (
          gGeneralOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
      }
      else {
#ifdef TRACE_OPTIONS
        if (gTraceOptions->fTraceGraceNotes || gTraceOptions->fTraceNotes) {
          fLogOutputStream <<
            "Appending grace note '" <<
            fCurrentGraceNoteClone->asShortString () <<
            "' to the grace notes group'" <<
            fCurrentGraceNotesGroupClone->asShortString () <<
            "' in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
#endif
  
        fCurrentGraceNotesGroupClone->
          appendNoteToGraceNotesGroup (
            fCurrentGraceNoteClone);
      }
      
    /* JMI ???
      if (fCurrentGraceNotesGroupClone) {
#ifdef TRACE_OPTIONS
        if (gTraceOptions->fTraceGraceNotes || gTraceOptions->fTraceNotes) {
          fLogOutputStream <<
            "Appending note '" <<
            fCurrentNonGraceNoteClone->asShortString () <<
            "' to the grace notes in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
#endif
  
        fCurrentGraceNotesClone->
          appendNoteToGraceNotes (
            fCurrentNonGraceNoteClone);
      }

      else if (fPendingAfterGraceNotes) {
#ifdef TRACE_OPTIONS
        if (gTraceOptions->fTraceGraceNotes || gTraceOptions->fTraceNotes) {
          fLogOutputStream <<
            "Appending note '" <<
            fCurrentNonGraceNoteClone->asShortString () <<
            "' to the after grace notes in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
#endif
  
        fPendingAfterGraceNotes->
          appendNoteToAfterGraceNotesContents (
            fCurrentNonGraceNoteClone);
      }
      
      else {
        stringstream s;

        s <<
          "both fCurrentGraceNoteGroupsClone and fPendingAfterGraceNoteGroup are null," <<
          endl <<
          "cannot handle grace note'" <<
          elt->asString () <<
          "'";

        msrInternalError (
          gGeneralOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
      }
      */
      break;
      
    case msrNote::kChordMemberNote:
      if (fOnGoingChord) {
        fCurrentChordClone->
          addAnotherNoteToChord (
            fCurrentNonGraceNoteClone,
            fCurrentVoiceClone);
      }
      
      else {
        stringstream s;

        s <<
          "msr2LpsrTranslator:::visitEnd (S_msrNote& elt): chord member note " <<
          elt->asString () <<
          " appears outside of a chord";

        msrInternalError (
          gGeneralOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
        }
      break;

    case msrNote::kGraceChordMemberNote:
      if (fOnGoingChord) {
        fCurrentChordClone->
          addAnotherNoteToChord (
            fCurrentGraceNoteClone,
            fCurrentVoiceClone);
      }
      
      else {
        stringstream s;

        s <<
          "msr2LpsrTranslator:::visitEnd (S_msrNote& elt): chord member note " <<
          elt->asString () <<
          " appears outside of a chord";

        msrInternalError (
          gGeneralOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
        }
      break;
      
    case msrNote::kTupletMemberNote:
    case msrNote::kGraceTupletMemberNote:
    case msrNote::kTupletMemberUnpitchedNote:
#ifdef TRACE_OPTIONS
      if (gTraceOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending note clone '" <<
          fCurrentNonGraceNoteClone->asShortString () << "'' to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
#endif
          
      fTupletClonesStack.top ()->
        addNoteToTuplet (
          fCurrentNonGraceNoteClone,
          fCurrentVoiceClone);
      break;
  } // switch

  // handle editorial accidentals
  switch (fCurrentNonGraceNoteClone->getNoteEditorialAccidentalKind ()) {
    case msrNote::kNoteEditorialAccidentalYes:
      fLpsrScore->
        // this score needs the 'editorial accidental' Scheme function
        setEditorialAccidentalSchemeFunctionIsNeeded ();
      break;
    case msrNote::kNoteEditorialAccidentalNo:
      break;
  } // switch
  
  // handle cautionary accidentals
  switch (fCurrentNonGraceNoteClone->getNoteCautionaryAccidentalKind ()) {
    case msrNote::kNoteCautionaryAccidentalYes:
      break;
    case msrNote::kNoteCautionaryAccidentalNo:
      break;
  } // switch

/* JMI
  // handle melisma
  msrSyllable::msrSyllableExtendKind
    noteSyllableExtendKind =
      elt->getNoteSyllableExtendKind ();

  switch (noteSyllableExtendKind) {
    case msrSyllable::kStandaloneSyllableExtend:
      {
        / * JMI ???
        // create melisma start command
        S_lpsrMelismaCommand
          melismaCommand =
            lpsrMelismaCommand::create (
              inputLineNumber,
              lpsrMelismaCommand::kMelismaStart);
    
        // append it to current voice clone
        fCurrentVoiceClone->
          appendOtherElementToVoice (melismaCommand);

        // append
        * /

        fOnGoingSyllableExtend = true;
      }
      break;
    case msrSyllable::kStartSyllableExtend:
      break;
    case msrSyllable::kContinueSyllableExtend:
      break;
    case msrSyllable::kStopSyllableExtend:
      break;
    case msrSyllable::k_NoSyllableExtend:
      break;
  } // switch
*/

  fOnGoingNote = false;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrOctaveShift& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOctaveShift" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    setNoteOctaveShift (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrOctaveShift& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOctaveShift" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrAccordionRegistration& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrAccordionRegistration" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the accordion registration to the voice clone
  fCurrentVoiceClone->
    appendAccordionRegistrationToVoice (elt);

  // the generated code needs modules scm and accreg
  fLpsrScore->
    setScmAndAccregSchemeModulesAreNeeded ();
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrHarpPedalsTuning& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrHarpPedalsTuning" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // append the harp pedals tuning to the voice clone
  fCurrentVoiceClone->
    appendHarpPedalsTuningToVoice (elt);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStem& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStem" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      setNoteStem (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      appendStemToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrStem& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrStem" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrBeam& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBeam" <<
      ", line " << elt->getInputLineNumber () <<
// JMI      ", fOnGoingNote = " << booleanAsString (fOnGoingNote) <<
// JMI      ", fOnGoingChord = " << booleanAsString (fOnGoingChord) <<
      endl;
  }
#endif

  // a beam may be present at the same time
  // in a note or grace note and the chord the latter belongs to
  
  if (fOnGoingGraceNotesGroup) {
    fCurrentGraceNoteClone->
      appendBeamToNote (elt);
  }
  else if (fOnGoingNote) {
    fCurrentNonGraceNoteClone->
      appendBeamToNote (elt);
  }

  if (fOnGoingChord) {
    fCurrentChordClone->
      appendBeamToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrBeam& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBeam" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrChord& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrChord" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fCurrentChordClone =
    elt->createChordNewbornClone (
      fCurrentPartClone);

  if (fTupletClonesStack.size ()) {
    // a chord in a tuplet is handled as part of the tuplet JMI
    fTupletClonesStack.top ()->
      addChordToTuplet (
        fCurrentChordClone);
  }

  else if (fOnGoingDoubleTremolo) {
    if (elt->getChordIsFirstChordInADoubleTremolo ()) {
      // replace double tremolo's first element by chord
      fCurrentDoubleTremoloClone->
        setDoubleTremoloChordFirstElement (
          elt);
    }
    
    else if (elt->getChordIsSecondChordInADoubleTremolo ()) {
      // replace double tremolo's second element by chord
      fCurrentDoubleTremoloClone->
        setDoubleTremoloChordSecondElement (
          elt);
    }
    
    else {
      stringstream s;

      s <<
        "chord '" << elt->asString () <<
        "' belongs to a double tremolo, but is not marked as such";

      msrInternalError (
        gGeneralOptions->fInputSourceName,
        inputLineNumber,
        __FILE__, __LINE__,
        s.str ());
    }
  }
  
  else if (fCurrentGraceNotesGroupClone) {
    // append the chord to the grace notes
    fCurrentGraceNotesGroupClone->
      appendChordToGraceNotesGroup (
        fCurrentChordClone);
  }
  
  else {
    // appending the chord to the voice clone at once
    fCurrentVoiceClone->
      appendChordToVoice (
        fCurrentChordClone);
  }

  fOnGoingChord = true;
}

void msr2LpsrTranslator::visitEnd (S_msrChord& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrChord" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fOnGoingChord = false;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTuplet& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTuplet" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  // create the tuplet clone
  S_msrTuplet
    tupletClone =
      elt->createTupletNewbornClone ();

  // register it in this visitor
#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceTuplets) {
    fLogOutputStream <<
      "++> pushing tuplet '" <<
      tupletClone->asString () <<
      "' to tuplets stack" <<
      endl;
  }
#endif
  
  fTupletClonesStack.push (tupletClone);

  switch (elt->getTupletLineShapeKind ()) {
    case msrTuplet::kTupletLineShapeStraight:
    case msrTuplet::kTupletLineShapeCurved:
      fLpsrScore->
        // this score needs the 'tuplets curved brackets' Scheme function
        setTupletsCurvedBracketsSchemeFunctionIsNeeded ();   
      break;
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrTuplet& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTuplet" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceTuplets) {
    fLogOutputStream <<
      "Popping tuplet '" <<
      elt->asString () <<
      "' from tuplets stack" <<
      endl;
  }
#endif
      
  fTupletClonesStack.pop ();

  if (fTupletClonesStack.size ()) {
    // tuplet is a nested tuplet
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTraceTuplets) {
      fLogOutputStream <<
        "Adding nested tuplet '" <<
      elt->asString () <<
        "' to stack top tuplet '" <<
      fTupletClonesStack.top ()->asString () <<
      "'" <<
      endl;
    }
#endif
    
    fTupletClonesStack.top ()->
      addTupletToTupletClone (elt);
  }
  
  else {
    // tuplet is a top level tuplet
    
#ifdef TRACE_OPTIONS
    if (gTraceOptions->fTraceTuplets) {
      fLogOutputStream <<
        "Adding top level tuplet '" <<
      elt->asString () <<
      "' to voice" <<
      fCurrentVoiceClone->getVoiceName () <<
      endl;
    }
#endif
      
    fCurrentVoiceClone->
      appendTupletToVoice (elt);
  }  
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTie& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTie" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    setNoteTie (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrTie& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTie" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSegno& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSegno" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendSegnoToVoice (elt);
}

void msr2LpsrTranslator::visitStart (S_msrCoda& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCoda" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendCodaToVoice (elt);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrEyeGlasses& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting eyeGlasses" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendEyeGlassesToNote (elt);
}

void msr2LpsrTranslator::visitStart (S_msrScordatura& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting scordatura" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendScordaturaToNote (elt);
}

void msr2LpsrTranslator::visitStart (S_msrPedal& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting pedal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendPedalToNote (elt);
}

void msr2LpsrTranslator::visitStart (S_msrDamp& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting damp" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendDampToNote (elt);

  fLpsrScore->
    // this score needs the 'custom short barline' Scheme function
    setDampMarkupIsNeeded ();
}

void msr2LpsrTranslator::visitStart (S_msrDampAll& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting dampAll" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentNonGraceNoteClone->
    appendDampAllToNote (elt);

  fLpsrScore->
    // this score needs the 'custom short barline' Scheme function
    setDampAllMarkupIsNeeded ();
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrBarCheck& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendBarCheckToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrBarCheck& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrBarNumberCheck& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarNumberCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendBarNumberCheckToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrBarNumberCheck& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarNumberCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrLineBreak& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrLineBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendLineBreakToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrLineBreak& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrLineBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPageBreak& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPageBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    appendPageBreakToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrPageBreak& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPageBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Handling repeat start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatStartInVoiceClone (
      inputLineNumber,
      elt);
}

void msr2LpsrTranslator::visitEnd (S_msrRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Handling repeat end in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
//      "\" in part \"" <<
//      fCurrentPartClone->getPartCombinedName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRepeatCommonPart& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeatCommonPart" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatCommonPartStartInVoiceClone (
      inputLineNumber);
}

void msr2LpsrTranslator::visitEnd (S_msrRepeatCommonPart& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeatCommonPart" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatCommonPartEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRepeatEnding& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeatEnding" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // handle the repeat ending start in the voice clone
#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Handling a repeat ending start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatEndingStartInVoiceClone (
      inputLineNumber,
      elt->getRepeatEndingKind (),
      elt->getRepeatEndingNumber ());
}

void msr2LpsrTranslator::visitEnd (S_msrRepeatEnding& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeatEnding" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // handle the repeat ending end in the voice clone
#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Handling a repeat ending end in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRepeatEndingEndInVoiceClone (
      inputLineNumber,
      elt->getRepeatEndingNumber (),
      elt->getRepeatEndingKind ());
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRestMeasures& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRestMeasures" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceRestMeasures) {
    fLogOutputStream <<
      "Handling multiple rest start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRestMeasuresStartInVoiceClone (
      inputLineNumber,
      elt);
}

void msr2LpsrTranslator::visitEnd (S_msrRestMeasures& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRestMeasures" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceRestMeasures) {
    fLogOutputStream <<
      "Handling multiple rest start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleRestMeasuresEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRestMeasuresContents& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRestMeasuresContents" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OPTIONS
  if (
    gTraceOptions->fTraceRestMeasures
      ||
    gTraceOptions->fTraceVoicesDetails
  ) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitStart (S_msrRestMeasuresContents&)");
  }
#endif

  fCurrentVoiceClone->
    handleRestMeasuresContentsStartInVoiceClone (
      inputLineNumber);
}

void msr2LpsrTranslator::visitEnd (S_msrRestMeasuresContents& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRestMeasuresContents" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;

#ifdef TRACE_OPTIONS
  if (
    gTraceOptions->fTraceRestMeasures
      ||
    gTraceOptions->fTraceVoicesDetails
  ) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitEnd (S_msrRestMeasuresContents&) 1");
  }
#endif

  fCurrentVoiceClone->
    handleRestMeasuresContentsEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMeasuresRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceMeasuresRepeats) {
    fLogOutputStream <<
      "Handling measures repeat start in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatStartInVoiceClone (
      inputLineNumber,
      elt);
}

void msr2LpsrTranslator::visitEnd (S_msrMeasuresRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasuresRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;

/* JMI
  // set last segment as the measures repeat pattern segment
#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceMeasuresRepeats) {
    fLogOutputStream <<
      "Setting current last segment as measures repeat pattern segment in voice \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif
*/

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceMeasuresRepeats) {
    fLogOutputStream <<
      "Handling measures repeat end in voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMeasuresRepeatPattern& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeatPattern" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OPTIONS
  if (
    gTraceOptions->fTraceMeasuresRepeats
      ||
    gTraceOptions->fTraceVoicesDetails
  ) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitStart (S_msrMeasuresRepeatPattern&)");
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatPatternStartInVoiceClone (
      inputLineNumber);
}

void msr2LpsrTranslator::visitEnd (S_msrMeasuresRepeatPattern& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasuresRepeatPattern" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;

#ifdef TRACE_OPTIONS
  if (
    gTraceOptions->fTraceMeasuresRepeats
      ||
    gTraceOptions->fTraceVoicesDetails
  ) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitEnd (S_msrMeasuresRepeatPattern&) 1");
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatPatternEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMeasuresRepeatReplicas& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeatReplicas" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

#ifdef TRACE_OPTIONS
  if (
    gTraceOptions->fTraceMeasuresRepeats
      ||
    gTraceOptions->fTraceVoicesDetails
  ) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitStart (S_msrMeasuresRepeatReplicas&)");
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatReplicasStartInVoiceClone (
      inputLineNumber);
}

void msr2LpsrTranslator::visitEnd (S_msrMeasuresRepeatReplicas& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting S_msrMeasuresRepeatReplicas" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter--;


#ifdef TRACE_OPTIONS
  if (
    gTraceOptions->fTraceMeasuresRepeats
      ||
    gTraceOptions->fTraceVoicesDetails
  ) {
    fCurrentVoiceClone->
      displayVoice (
        inputLineNumber,
        "Upon visitEnd (S_msrMeasuresRepeatReplicas&) 1");
  }
#endif

  fCurrentVoiceClone->
    handleMeasuresRepeatReplicasEndInVoiceClone (
      inputLineNumber);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrBarline& elt)
{
#ifdef TRACE_OPTIONS
  int inputLineNumber =
    elt->getInputLineNumber ();
#endif
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarline" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

#ifdef TRACE_OPTIONS
  if (gTraceOptions->fTraceBarlines) {
    fLogOutputStream <<
      "Handling '" <<
      msrBarline::barlineCategoryKindAsString (
        elt->getBarlineCategory ()) <<
      "' in voice \"" <<
      fCurrentVoiceClone->getVoiceName () << "\"" <<
      endl;
  }
#endif

  switch (elt->getBarlineStyleKind ()) {
    case msrBarline::kBarlineStyleNone:
      break;
    case msrBarline::kBarlineStyleRegular:
      break;
    case msrBarline::kBarlineStyleDotted:
      break;
    case msrBarline::kBarlineStyleDashed:
      break;
    case msrBarline::kBarlineStyleHeavy:
      break;
    case msrBarline::kBarlineStyleLightLight:
      break;
    case msrBarline::kBarlineStyleLightHeavy:
      break;
    case msrBarline::kBarlineStyleHeavyLight:
      break;
    case msrBarline::kBarlineStyleHeavyHeavy:
      break;
    case msrBarline::kBarlineStyleTick:
      break;
    case msrBarline::kBarlineStyleShort:
      fLpsrScore->
        // this score needs the 'custom short barline' Scheme function
        setCustomShortBarLineSchemeFunctionIsNeeded ();
      break;
      /* JMI
    case msrBarline::kBarlineStyleNone:
      break;
      */
  } // switch

  // append the barline to the current voice clone
  fCurrentVoiceClone->
    appendBarlineToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrBarline& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarline" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrVarValAssoc& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVarValAssoc" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  msrVarValAssoc::msrVarValAssocKind
    varValAssocKind =
      elt->getVarValAssocKind ();
  string variableValueAux = elt->getVariableValue ();
  string variableValue;

  // escape quotes if any
  for_each (
    variableValueAux.begin (),
    variableValueAux.end (),
    stringQuoteEscaper (variableValue));

  switch (varValAssocKind) {
    case msrVarValAssoc::kWorkNumber:
      fCurrentIdentification->
        setWorkNumber (
          inputLineNumber, variableValue);
      
      fLpsrScoreHeader->
        setWorkNumber (
          inputLineNumber,
          variableValue,
          kFontStyleNone,
          kFontWeightNone);
  
      fWorkNumberKnown = true;
      break;
  
    case msrVarValAssoc::kWorkTitle:
      fCurrentIdentification->
        setWorkTitle (
          inputLineNumber, variableValue);
      
      fLpsrScoreHeader->
        setWorkTitle (
          inputLineNumber,
          variableValue,
          kFontStyleNone,
          kFontWeightNone);
          
      fWorkTitleKnown = true;
      break;
  
    case msrVarValAssoc::kMovementNumber:
      fCurrentIdentification->
        setMovementNumber (
          inputLineNumber, variableValue);
      
      fLpsrScoreHeader->
        setMovementNumber (
          inputLineNumber,
          variableValue,
          kFontStyleNone,
          kFontWeightNone);
  
      fMovementNumberKnown = true;
      break;
  
    case msrVarValAssoc::kMovementTitle:
      fCurrentIdentification->
        setMovementTitle (
          inputLineNumber, variableValue);
      
      fLpsrScoreHeader->
        setMovementTitle (
          inputLineNumber,
          variableValue,
          kFontStyleNone,
          kFontWeightNone);
          
      fMovementTitleKnown = true;
      break;
  
    case msrVarValAssoc::kEncodingDate:
      fCurrentIdentification->
        setEncodingDate (
          inputLineNumber, variableValue);
      
      fLpsrScoreHeader->
        setEncodingDate (
          inputLineNumber,
          variableValue,
          kFontStyleNone,
          kFontWeightNone);
      break;

    case msrVarValAssoc::kScoreInstrument:
      fCurrentIdentification->
        setScoreInstrument (
          inputLineNumber, variableValue);
      
      fLpsrScoreHeader->
        setScoreInstrument (
          inputLineNumber,
          variableValue,
          kFontStyleNone,
          kFontWeightNone);
      break;

    case msrVarValAssoc::kMiscellaneousField:
      fCurrentIdentification->
        setMiscellaneousField (
          inputLineNumber, variableValue);
      
      fLpsrScoreHeader->
        setMiscellaneousField (
          inputLineNumber,
          variableValue,
          kFontStyleNone,
          kFontWeightNone);
      break;

    default:
      {
      stringstream s;
    
      s <<
        "### msrVarValAssoc kind '" <<
        msrVarValAssoc::varValAssocKindAsString (
          varValAssocKind) <<
        "' is not handled";
    
      msrMusicXMLWarning (
        gGeneralOptions->fInputSourceName,
        inputLineNumber,
        s.str ());
      }
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrVarValAssoc& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrVarValAssoc" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrVarValsListAssoc& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVarValsListAssoc" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  msrVarValsListAssoc::msrVarValsListAssocKind
    varValsListAssocKind =
      elt->getVarValsListAssocKind ();

  const list<string>&
    variableValuesList =
      elt->getVariableValuesList ();

  switch (varValsListAssocKind) {
    case msrVarValsListAssoc::kRights:
      for (list<string>::const_iterator i = variableValuesList.begin ();
        i != variableValuesList.end ();
        i++) {
        fLpsrScoreHeader->
          addRights (
            inputLineNumber, (*i));
      } // for
      break;
  
    case msrVarValsListAssoc::kComposer:
      for (list<string>::const_iterator i = variableValuesList.begin ();
        i != variableValuesList.end ();
        i++) {
        fLpsrScoreHeader->
          addComposer (
            inputLineNumber, (*i));
      } // for
      break;
  
    case msrVarValsListAssoc::kArranger:
      for (list<string>::const_iterator i = variableValuesList.begin ();
        i != variableValuesList.end ();
        i++) {
        fLpsrScoreHeader->
          addArranger (
            inputLineNumber, (*i));
      } // for
      break;
  
    case msrVarValsListAssoc::kLyricist:
      for (list<string>::const_iterator i = variableValuesList.begin ();
        i != variableValuesList.end ();
        i++) {
        fLpsrScoreHeader->
          addLyricist (
            inputLineNumber, (*i));
      } // for
      break;

    case msrVarValsListAssoc::kPoet:
      for (list<string>::const_iterator i = variableValuesList.begin ();
        i != variableValuesList.end ();
        i++) {
        fLpsrScoreHeader->
          addPoet (
            inputLineNumber, (*i));
      } // for
      break;
  
    case msrVarValsListAssoc::kTranslator:
      for (list<string>::const_iterator i = variableValuesList.begin ();
        i != variableValuesList.end ();
        i++) {
        fLpsrScoreHeader->
          addTranslator (
            inputLineNumber, (*i));
      } // for
      break;

    case msrVarValsListAssoc::kSoftware:
      for (list<string>::const_iterator i = variableValuesList.begin ();
        i != variableValuesList.end ();
        i++) {
        fLpsrScoreHeader->
          addSoftware (
            inputLineNumber, (*i));
      } // for
      break;
/* JMI
    default:
      {
      stringstream s;
    
      s <<
        "### msrVarValsListAssoc kind '" <<
        msrVarValsListAssoc::varValsListAssocKindAsString (
          varValsListAssocKind) <<
        "' is not handled";
   
      msrMusicXMLWarning (
        gGeneralOptions->fInputSourceName,
        inputLineNumber,
        s.str ());
      }
      */
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrVarValsListAssoc& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrVarValsListAssoc" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrLayout& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrLayout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif

  gIndenter++;
}

void msr2LpsrTranslator::visitEnd (S_msrLayout& elt)
{
  gIndenter--;

#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrLayout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMidi& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMidi" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}

void msr2LpsrTranslator::visitEnd (S_msrMidi& elt)
{
#ifdef TRACE_OPTIONS
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMidi" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
#endif
}


} // namespace


/* JMI
 *   fCurrentVoiceClone =
    elt->createVoiceNewbornClone (fCurrentStaffClone);
    
  fCurrentStaffClone->
    registerVoiceInStaff (fCurrentVoiceClone);

  // append the voice to the LPSR score elements list
  fLpsrScore ->
    appendVoiceToScoreElements (fCurrentVoiceClone);

  // append the voice use to the LPSR score block
  fLpsrScore ->
    appendVoiceUseToStoreBlock (fCurrentVoiceClone);
*/





           /* JMI   
  fLogOutputStream <<
    endl <<
    "*********** fCurrentPartClone" <<
    endl <<
    endl;
  fCurrentPartClone->print (fLogOutputStream);
  fLogOutputStream <<
    "*********** fCurrentPartClone" <<
    endl <<
    endl;
    */
/* JMI
  fLogOutputStream <<
    endl <<
    "*********** fCurrentRepeatClone" <<
    endl <<
    endl;
  fCurrentRepeatClone->print (fLogOutputStream);
  fLogOutputStream <<
    "*********** fCurrentRepeatClone" <<
    endl <<
    endl;
*/

