/*
 MusicXML Library
 Copyright (C) Grame 2006-2013
 
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 
 Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
 research@grame.fr
 */

#ifdef MSVC
# pragma warning (disable : 4786)
#endif

#include <iostream>
#include <sstream>
#include <string>

#include "partsummary.h"
#include "rational.h"
#include "xml_tree_browser.h"
#include "xml2guidovisitor.h"
#include "xmlpart2guido.h"
#include "tree_browser.h"

using namespace std;


namespace MusicXML2
{
    int xml2guidovisitor::defaultStaffDistance = 0;
    
    //______________________________________________________________________________
    xml2guidovisitor::xml2guidovisitor(bool generateComments, bool generateStem, bool generateBar, int partNum,
                                       int beginMeasure, double beginMeasureOffset,
                                       int endMeasure, int endMeasureOffset, double endMeasureBeatOffset) :
    fGenerateComments(generateComments), fGenerateStem(generateStem),
    fGenerateBars(generateBar), fGeneratePositions(true),
    fCurrentStaffIndex(0), fCurrentPartStaffOffset(0), previousStaffHasLyrics(false), fCurrentAccoladeIndex(0), fPartNum(partNum),
    fBeginMeasure(beginMeasure), fBeginMeasureBeatOffset(beginMeasureOffset),
    fEndMeasure(endMeasure), fEndMeasureOffset(endMeasureOffset), fEndMeasureBeatOffset(endMeasureBeatOffset),
    fTotalMeasures(0), fTotalDuration(0.0)
    {
        fPartsAvailable = 0;
    }
    
    //______________________________________________________________________________
    Sguidoelement xml2guidovisitor::convert (const Sxmlelement& xml)
    {
        Sguidoelement gmn;
        if (xml) {
            tree_browser<xmlelement> browser(this);
            browser.browse(*xml);
            gmn = current();
        }
        return gmn;
    }
    
    //______________________________________________________________________________
    // the score header contains information like title, author etc..
    // it must be written only once, at the beginning of the first guido voice
    // thus the function clears the data when flushed so that further calls do nothing
    //______________________________________________________________________________
    void xml2guidovisitor::flushHeader ( scoreHeader& header )
    {
        if (header.fTitle) {
            Sguidoelement tag = guidotag::create("title");
            string title = header.fTitle->getValue();
            size_t pos = title.find ('"');
            while (pos != string::npos) {
                title = title.replace (pos, 1, "'");
                pos = title.find ('"', pos);
            }
            tag->add (guidoparam::create(title));
            add (tag);
            header.fTitle = (void*)0;
        }
        vector<S_creator>::const_iterator i;
        for (i=header.fCreators.begin(); i!=header.fCreators.end(); i++) {
            string type = (*i)->getAttributeValue("type");
            if ((type == "Composer") || (type == "composer")) {
                Sguidoelement tag = guidotag::create("composer");
                tag->add (guidoparam::create((*i)->getValue()));
                tag->add (guidoparam::create("dy=4hs", false));
                add (tag);
            }
        }
        header.fCreators.clear();
    }
    
    //______________________________________________________________________________
    // the part header contains information like part name
    // it must be written only once, at the beginning of the corresponding guido voice
    // thus the function clears the data when flushed so that further calls do nothing
    //______________________________________________________________________________
    void xml2guidovisitor::flushPartHeader ( partHeader& header )
    {
        if ( (header.visited==false) && header.fPartName.size()) {
            
            // As of Guido 1.64, we won't use derived dx/dy for \instr but use "\auto<autoInstrPos="on">" instead
            stringstream s1, s2, s0;
            
            Sguidoelement autoTag = guidotag::create("auto");
            s0 << "autoInstrPos=\"on\"";
            autoTag->add (guidoparam::create(s0.str(), false));
            add(autoTag);
            
            Sguidoelement tag = guidotag::create("instr");
            string instr = header.fPartName;
            
            //int offset = instr.size() * 2;
            /* removed as of GuidoLib 1.64
             s1 << "dx=-" << offset << "hs";
             tag->add (guidoparam::create(s1.str(), false));
             tag->add (guidoparam::create("dy=-5hs", false));
             */
            tag->add (guidoparam::create(instr));
            add (tag);
            
            
            /*tag = guidotag::create("systemFormat");
            tag->add (guidoparam::create(""));
            s2 << "dx=" << offset << "hs";
            tag->add (guidoparam::create(s2.str(), false));
            
            add (tag);*/
            header.visited = true;
        }
    }
    
    void xml2guidovisitor::flushPartGroup (std::string partID)
    {
        /// Add groupings (accolade and barformat)
        // search if this part ID exists in any grouping
        // Guido Limitation: One \accol tag per staff ONLY (for nested definitions)
        // IDEA: Treat the FIRST occurence of partID in grouping and get rid of it.
        partGroup* partGroupIt = find_first_of_partID_inGroup(partID);
        
        if (partGroupIt != NULL && partGroupIt->guidoRange.size()>0)
        {
            /// something was found. Generate Accolades and BarFormat if any
            int rangeStart = fCurrentStaffIndex ;
            int rangeEnd = rangeStart + (partGroupIt->guidoRangeStop - partGroupIt->guidoRangeStart);
            std::stringstream rangeFixed;
            rangeFixed << " range=\""<< rangeStart <<"-"<<rangeEnd<<"\"";
            
            if (partGroupIt->bracket)
            {
                std::stringstream accolParams;
                accolParams << "id=1, "<< rangeFixed.str();
                
                Sguidoelement tag3 = guidotag::create("accol");
                tag3->add (guidoparam::create(accolParams.str(), false));
                add (tag3);
            }
            
            if (partGroupIt->barlineGrouping)
            {
                std::string barformatParams = "style= \"system\", "+rangeFixed.str();
                
                Sguidoelement tag4 = guidotag::create("barFormat");
                tag4->add (guidoparam::create(barformatParams, false));
                add (tag4);
            }
            
            /// Make sure that this group pattern won't be visited next time
            partGroupIt->visited = true;
        }
    }
    
    //______________________________________________________________________________
    void xml2guidovisitor::visitStart ( S_score_partwise& elt )
    {
        Sguidoelement chord = guidochord ::create();
        start(chord);
    }
    
    void xml2guidovisitor::visitStart( S_defaults& elt)
    {
        xml2guidovisitor::defaultStaffDistance = elt->getIntValue(k_staff_distance, 0);
    }
    
    //______________________________________________________________________________
    void xml2guidovisitor::visitStart ( S_movement_title& elt )		{ fHeader.fTitle = elt; }
    void xml2guidovisitor::visitStart ( S_creator& elt )			{ fHeader.fCreators.push_back(elt); }
    //void xml2guidovisitor::visitStart ( S_score_part& elt )			{ fCurrentPartID = elt->getAttributeValue("id"); }
    //void xml2guidovisitor::visitStart ( S_part_name& elt )			{ fPartHeaders[fCurrentPartID].fPartName = elt; }
    
    //______________________________________________________________________________
    void xml2guidovisitor::visitStart ( S_part& elt )
    {
        currentPart = elt->getAttributeValue("id");
        stavesInPart[currentPart] = 1;
        fPartsAvailable++;
        
        // Filter out score-part here
        if (fPartNum != 0) {
            std::stringstream s;
            s << "P"<<fPartNum;
            std::string thisPart = elt->getAttributeValue("id");
            if ( thisPart != s.str() ) {
                return;
            }
        }
        
        partsummary ps;
        xml_tree_browser browser(&ps);
        browser.browse(*elt);
        
        smartlist<int>::ptr voices = ps.getVoices ();
        int targetStaff = -1;	// initialized to a value we'll unlikely encounter
        bool notesOnly = false;
        rational currentTimeSign (0,1);
        std::vector<int> processedDirections;

        // browse the parts voice by voice: allows to describe voices that spans over several staves
        for (unsigned int i = 0; i < voices->size(); i++) {
            int targetVoice = (*voices)[i];
            int mainstaff = ps.getMainStaff(targetVoice);
            if (targetStaff == -1) {
                notesOnly = false;
                targetStaff = mainstaff;
                fCurrentStaffIndex++;
                // Create intermediate staves if any.
                // This happens when we enter a Part and the first voice's mainStaff is NOT the first staff! Example: "Bonis 6 pieces for 4 hands part 3"
                while (fCurrentStaffIndex < targetStaff + fCurrentPartStaffOffset) {
                    createStaff(fCurrentStaffIndex, elt);
                    fCurrentStaffIndex++;
                }
            } else
            if (targetStaff == mainstaff) {
                notesOnly = true;
            }
            else {
                notesOnly = false;
                // Only create staff if mainStaff > targetStaff
                if (mainstaff > targetStaff) {
                    fCurrentStaffIndex++;
                }
                targetStaff = mainstaff;
                // Create intermediate staves if needed
                while (fCurrentStaffIndex < targetStaff+fCurrentPartStaffOffset) {
                    createStaff(fCurrentStaffIndex, elt);
                    fCurrentStaffIndex++;
                }
            }
            
            Sguidoelement seq = guidoseq::create();
            push (seq);
            
            Sguidoelement tag = guidotag::create("staff");
            tag->add (guidoparam::create(fCurrentStaffIndex, false));
            add (tag);
            
            // Add \set<autoHideTiedAccidentals="on"> to avoid repetition of accidentals on ties
            std::string autoHideTiedAccidentals = "set<autoHideTiedAccidentals=\"on\">";
            tag = guidotag::create(autoHideTiedAccidentals);
            add(tag);
            
            // Add  \set<resolveMultiVoiceCollisions="on">
            // NOTE: resolveMultiVoiceCollisions will lead to bad beaming in multi-voice scores!
//            if (fCurrentStaffIndex == 1) {
//                std::string resolveMultiVoiceCollisions = "set<resolveMultiVoiceCollisions=\"on\">";
//                tag = guidotag::create(resolveMultiVoiceCollisions);
//                add(tag);
//            }
                        
            /// Add staffFormat if needed
            // We do not infer default staff distance from musicXML since no software seem to be able to control it!
            int stafflines = elt->getIntValue(k_staff_lines, 0);
            if (stafflines)
            {
                Sguidoelement tag2 = guidotag::create("staffFormat");
                if (stafflines>0)
                {
                    stringstream staffstyle;
                    staffstyle << "style=\"" << stafflines<<"-line\"";
                    tag2->add (guidoparam::create(staffstyle.str(),false));
                }
                add (tag2);
            }
            
            flushHeader (fHeader);
            flushPartHeader (fPartHeaders[elt->getAttributeValue("id")]);
            flushPartGroup  (elt->getAttributeValue("id"));
            

            //// Add Accolade if countStaves on this Part is >1, and we are entering span
            if ((ps.countStaves()>1)&&(fCurrentStaffIndex>fCurrentAccoladeIndex)) {
                int rangeBegin = min(fCurrentStaffIndex, fCurrentPartStaffOffset+1);
                int rangeEnd = fCurrentPartStaffOffset + ps.countStaves();

				stringstream accol;
				accol << "id=" << fCurrentAccoladeIndex << ", range=\"" << rangeBegin << "-" << rangeEnd << "\"";
				stringstream barformat;
				barformat << "style= \"system\", range=\"" << rangeBegin << "-" << rangeEnd << "\"";
				
                Sguidoelement tag3 = guidotag::create("accol");
                tag3->add (guidoparam::create(accol.str(), false));
                add (tag3);
                
                // add new barFormat tag
                Sguidoelement tag4 = guidotag::create("barFormat");
                tag4->add (guidoparam::create(barformat.str(), false));
                add (tag4);
				
                fCurrentAccoladeIndex = rangeEnd;
            }else {
                
                // if we have multiple staves, and this staff has NO barFormat, then we should add one like \barFormat<style="system", range="1">
                // Otherwise there'll be no bar lines!!!
                if (checkLonelyBarFormat(fCurrentStaffIndex))
                {
					stringstream barformat;
					barformat << "style= \"system\", range=\"" << fCurrentStaffIndex << "\"";
                    Sguidoelement tag4 = guidotag::create("barFormat");
                    tag4->add (guidoparam::create(barformat.str(), false));
                    add (tag4);
                }
            }
            
            /// If this Voice does NOT have any clef we must add.. (TODO: Check!!!)
            if (notesOnly && fBeginMeasure <= 1) {
                auto iter = elt->find(k_clef, elt->begin());
                if (iter != elt->end()) {
                    parseClef(iter, fCurrentStaffIndex);
                }
            }
                        
            /// Browse XML and convert
            //cerr<<"<<< Browse staff "<<targetStaff<<" voice:"<< targetVoice<<endl;
            xmlpart2guido pv(fGenerateComments, fGenerateStem, fGenerateBars,
                             fBeginMeasure, fBeginMeasureBeatOffset,
                             fEndMeasure, fEndMeasureOffset, fEndMeasureBeatOffset);
            pv.generatePositions (fGeneratePositions);
            xml_tree_browser browser(&pv);
            pv.initialize(seq, targetStaff, fCurrentStaffIndex, targetVoice, notesOnly, currentTimeSign);
            pv.octavas = ps.fOctavas[targetStaff];
            pv.processedDirections = processedDirections;
            pv.timePositions = ps.timePositions;
            browser.browse(*elt);
            // Add an explicite \endBar on the targetStaff
            Sguidoelement tagStaff = guidotag::create("staff");
            tagStaff->add (guidoparam::create(fCurrentStaffIndex, false));
            add (tagStaff);
            Sguidoelement tagEndBar = guidotag::create("endBar");
            add (tagEndBar);
            pop(); // Close staff
            currentTimeSign = pv.getTimeSign();
            previousStaffHasLyrics = pv.hasLyrics();
            if (fBeginPosition.toDouble() < pv.fStartPosition.toDouble()) {
                fBeginPosition = pv.fStartPosition;
            }
            fEndPosition = pv.fEndPosition;
            processedDirections.insert(processedDirections.end(), pv.processedDirections.begin(), pv.processedDirections.end());
            
            if (pv.lastMeasureNumber() > fTotalMeasures) {
                fTotalMeasures = pv.lastMeasureNumber();
            }
            measurePositionMap = pv.measurePositionMap;
            timeSignatureMap = pv.timeSignatureMap;
            
            if (pv.totalPartDuration() > fTotalDuration) {
                fTotalDuration = pv.totalPartDuration();
            }
        }
        
        // Check for implicit staves in MusicXML and create them.
        while (fCurrentStaffIndex < fCurrentPartStaffOffset + ps.countStaves()) {
            fCurrentStaffIndex++;
            createStaff(fCurrentStaffIndex, elt);
        }
        fCurrentPartStaffOffset += ps.countStaves();
    }

void xml2guidovisitor::createStaff(int staffIndex, S_part& elt) {
    Sguidoelement seq = guidoseq::create();
    push (seq);
    
    Sguidoelement tag = guidotag::create("staff");
    tag->add (guidoparam::create(staffIndex, false));
    add (tag);
    
    // Add clefs
    auto iter = elt->find(k_clef, elt->begin());
    while (iter != elt->end()) {
        parseClef(iter, staffIndex);
        iter = elt->find(k_clef, iter++);
    }
    
    // Add Key
    iter = elt->find(k_key, elt->begin());
    while (iter != elt->end()) {
        string keymode = iter->getValue(k_mode);
        int keyfifths = iter->getIntValue(k_fifths, 0);
        Sguidoelement tag = guidotag::create("key");
        tag->add (guidoparam::create(keyfifths, false));
        add (tag);
        iter = elt->find(k_key, iter++);
    }
    
    // Add Meter
    iter = elt->find(k_time, elt->begin());
    if (iter != elt->end()) {
        parseTime(iter);
        //iter = elt->find(k_key, iter++);
    }
    
    pop();
}

void xml2guidovisitor::parseClef(ctree<xmlelement>::iterator &iter, int staffIndex) {
    // Check that clef belongs to this staff
    if (iter->getAttributeIntValue("number", 1) == staffIndex) {
        string clefsign = iter->getValue(k_sign);
        int clefline = iter->getIntValue(k_line, 0);
        int clefoctavechange = iter->getIntValue(k_clef_octave_change, 0);
                            
        stringstream s;
        if ( clefsign == "G")            s << "g";
        else if ( clefsign == "F")    s << "f";
        else if ( clefsign == "C")    s << "c";
        else if ( clefsign == "percussion")    s << "perc";
        else if ( clefsign == "TAB")    s << "TAB";
        else if ( clefsign == "none")    s << "none";
        else {                                                    // unknown clef sign !!
            cerr << "warning: unknown clef sign \"" << clefsign << "\"" << endl;
            return;
        }
        
        string param;
        if (clefline != 0)  // was clefvisitor::kStandardLine
            s << clefline;
        s >> param;
        if (clefoctavechange == 1)
            param += "+8";
        else if (clefoctavechange == -1)
            param += "-8";
        
        
        Sguidoelement tag = guidotag::create("clef");
        tag->add (guidoparam::create(param));
        add(tag);
    }
}
    
void xml2guidovisitor::parseTime(ctree<xmlelement>::iterator &iter) {
    bool senzamesura = (iter->find(k_senza_misura) != iter->end());
    string timesymbol = iter->getAttributeValue("symbol");
    std::vector<std::pair<std::string,std::string> > fTimeSignInternal ;
    rational fCurrentTimeSign;
    
    // IOSEPRAC-185: Get all pairs for Composite Time Signatures
    ctree<xmlelement>::iterator iter_beat = iter->find(k_beats);
    ctree<xmlelement>::iterator iter_beatType = iter->find(k_beat_type);
    
    while (iter_beat != iter->end())
    {
        fTimeSignInternal.push_back(make_pair(iter_beat->getValue(),
                                              iter_beatType->getValue()));
        iter_beat = iter->find(k_beats, iter_beat++);
        iter_beatType = iter->find(k_beat_type, iter_beatType++);
    }
    
    /// Actions:
    string timesign;
    if (!senzamesura) {
        if (timesymbol == "common") {
            size_t index = 0;
            rational ts(0,1);
            if (index < fTimeSignInternal.size()) {
                const pair<string,string>& tss = fTimeSignInternal[index];
                long num = strtol (tss.first.c_str(), 0, 10);
                long denum = strtol (tss.second.c_str(), 0, 10);
                if (num && denum) ts.set(num, denum);
            }
            ///
            if ((ts.getDenominator() == 2) && (ts.getNumerator() == 2))
                timesign = "C/";
            else if ((ts.getDenominator() == 4) && (ts.getNumerator() == 4))
                timesign = "C";
            else
                timesign = string(ts);
            fCurrentTimeSign = ts;
        }
        else if (timesymbol == "cut") {
            timesign = "C/";
            fCurrentTimeSign = rational(2,2);
        }
        else {
            stringstream s; string sep ="";
            fCurrentTimeSign.set(0,1);
            for (unsigned int i = 0; i < fTimeSignInternal.size(); i++) {
                s << sep << fTimeSignInternal[i].first << "/" << fTimeSignInternal[i].second;
                sep = "+";
                size_t index = i;
                rational ts(0,1);
                if (index < fTimeSignInternal.size()) {
                    const pair<string,string>& tss = fTimeSignInternal[index];
                    long num = strtol (tss.first.c_str(), 0, 10);
                    long denum = strtol (tss.second.c_str(), 0, 10);
                    if (num && denum) ts.set(num, denum);
                }
                ///
                fCurrentTimeSign += ts;
            }
            s >> timesign;
        }
    }
    
    Sguidoelement tag = guidotag::create("meter");
    tag->add (guidoparam::create(timesign));
    if (fGenerateBars)
        tag->add (guidoparam::create("autoBarlines=\"off\"", false));
    if (true)
        tag->add (guidoparam::create("autoMeasuresNum=\"system\"", false));
    
    if (iter->getAttributeValue("print-object")!="no") {
        add(tag);
    }
}

    //______________________________________________________________________________
    void xml2guidovisitor::addPosition	( Sxmlelement elt, Sguidoelement& tag, float yoffset)
    {
        float posx = elt->getAttributeFloatValue("default-x", 0) + elt->getAttributeFloatValue("relative-x", 0);
        if (posx) {
            posx = (posx / 10) * 2;   // convert to half spaces
            stringstream s;
            s << "dx=" << posx << "hs";
            tag->add (guidoparam::create(s.str(), false));
        }
        float posy = elt->getAttributeFloatValue("default-y", 0) + elt->getAttributeFloatValue("relative-y", 0);
        if (posy) {
            posy = (posy / 10) * 2;   // convert to half spaces
            posy += yoffset;		  // anchor point convertion (defaults to upper line in xml)
            stringstream s;
            s << "dy=" << posy << "hs";
            tag->add (guidoparam::create(s.str(), false));
        }
    }
    
    //______________________________________________________________________________
    /**
     Infers x and y position from MusicXML element, converts to Guido HS and adds to tag

     @param elt MusicXML element with x-y position
     @param tag Guido Tag in construction
     @param yoffset y Offset to add to converted HS result
     @param xoffset x Offset to add to converted HS result
     */
    void xml2guidovisitor::addPosition	( Sxmlelement elt, Sguidoelement& tag, float yoffset, float xoffset)
    {
        float posx = elt->getAttributeFloatValue("default-x", 0) + elt->getAttributeFloatValue("relative-x", 0);
        if (posx) {
            posx = (posx / 10) * 2;   // convert to half spaces
            posx += xoffset;
            stringstream s;
            s << "dx=" << posx << "hs";
            tag->add (guidoparam::create(s.str(), false));
        }
        float posy = elt->getAttributeFloatValue("default-y", 0) + elt->getAttributeFloatValue("relative-y", 0);
        if (posy) {
            posy = (posy / 10) * 2;   // convert to half spaces
            posy += yoffset;		  // anchor point convertion (defaults to upper line in xml)
            stringstream s;
            s << "dy=" << posy << "hs";
            tag->add (guidoparam::create(s.str(), false));
        }
    }
    
    //______________________________________________________________________________
    
    /**
     Infers MusicXML Y-Position of element and converts to Guido Half-Space
     
     @param elt XML Element visiting
     @param tag The Guido Tag element to append dY position
     @param yoffset Half-space offset to add
     @param .0 Multiplier: can be 1.0 or -1.0 (to invert)
     */
    void xml2guidovisitor::addPosY	( Sxmlelement elt, Sguidoelement& tag, float yoffset, float ymultiplier = 1.0)
    {
        float posy = elt->getAttributeFloatValue("relative-y", 0) + elt->getAttributeFloatValue("default-y", 0);
            posy = (posy / 10) * 2;   // convert to half spaces
            posy += yoffset;		  // anchor point convertion (defaults to upper line in xml)
            posy = posy * ymultiplier;
        if (posy) {
            stringstream s;
            s << "dy=" << posy << "hs";
            tag->add (guidoparam::create(s.str(), false));
        }
    }
    
    //______________________________________________________________________________
    void xml2guidovisitor::addPosX    ( Sxmlelement elt, Sguidoelement& tag, float xoffset)
    {
        float posx = elt->getAttributeFloatValue("default-x", 0) + elt->getAttributeFloatValue("relative-x", 0);
        
        posx = (posx / 10) * 2;   // convert to half spaces
        posx += xoffset;          // anchor point convertion (defaults to upper line in xml)
        if (posx) {
            stringstream s;
            s << "dx=" << posx << "hs";
            tag->add (guidoparam::create(s.str(), false));
        }
    }

void xml2guidovisitor::addRelativeX(Sxmlelement elt, Sguidoelement& tag, float xoffset){
    float posx = elt->getAttributeFloatValue("relative-x", 0.0);
    if (posx == 0.0) {
        return;
    }
    posx = (posx / 10) * 2;   // convert to half spaces
    posx += xoffset;
    
    stringstream s;
    s << "dx=" << posx << "hs";
    tag->add (guidoparam::create(s.str(), false));
    
}
    
    void xml2guidovisitor::addPlacement	( Sxmlelement elt, Sguidoelement& tag)
    {
        string placement = elt->getAttributeValue("placement");
        
        if (placement.size()) {
            stringstream s;
            s << "position=\"" << placement << "\"";
            tag->add (guidoparam::create(s.str(), false));
        }
    }
    
    /**
     Converts total relative and default Y for element from MusicXML Tenths to Guido Half-Space

     @param elt Music XML element
     @param yoffset half-space offset to be added after conversion
     @param useDefault true to use default-y position, false otherwise
     @return returns half-space in float
     */
    float xml2guidovisitor::getYposition(Sxmlelement elt, float yoffset, bool useDefault = true){
        float posy = (useDefault ? elt->getAttributeFloatValue("default-y", 0) : 0.0)
                + elt->getAttributeFloatValue("relative-y", 0);
        posy = (posy / 10) * 2;   // convert to half spaces
        posy += yoffset;          // anchor point convertion (defaults to upper line in xml)

        return posy;
    }
    
    float xml2guidovisitor::getXposition(Sxmlelement elt, float xoffset){
        float posx = elt->getAttributeFloatValue("default-x", 0) + elt->getAttributeFloatValue("relative-x", 0);
        posx = (posx / 10) * 2;   // convert to half spaces
        posx += xoffset;
        
        return posx;
    }

    /// direction attribute in MusicXML can be "up" or "down"
    void xml2guidovisitor::addDirection( Sxmlelement elt, Sguidoelement& tag)
    {
        string direction = elt->getAttributeValue("direction");
        
        if (direction.size()) {
            
            stringstream s;
            s << "direction=\"" << direction << "\"";
            tag->add (guidoparam::create(s.str(), false));
        }
    }

void xml2guidovisitor::visitStart ( S_staves& elt)
{
    stavesInPart[currentPart] = int(*elt);
}

void xml2guidovisitor::visitEnd ( S_clef& elt )
{
    std::string key;
    if(clefvisitor::fSign == "G" && clefvisitor::fLine == 2 && clefvisitor::fOctaveChange == 0) {
        key = "g2";
    }else if(clefvisitor::fSign == "G" && clefvisitor::fLine == 2 && clefvisitor::fOctaveChange == -1) {
        key = "g-8";
    }else if(clefvisitor::fSign == "G" && clefvisitor::fLine == 2 && clefvisitor::fOctaveChange == 1) {
        key = "g+8";
    }else if(clefvisitor::fSign == "F" && clefvisitor::fLine == 4 && clefvisitor::fOctaveChange == 0) {
        key = "f4";
    }else if(clefvisitor::fSign == "F" && clefvisitor::fLine == 4 && clefvisitor::fOctaveChange == -1) {
        key = "f-8";
    }else if(clefvisitor::fSign == "F" && clefvisitor::fLine == 4 && clefvisitor::fOctaveChange == 1) {
        key = "f+8";
    }else if(clefvisitor::fSign == "C" && clefvisitor::fLine == 3 && clefvisitor::fOctaveChange == 0) {
        key = "alto";
    }else if(clefvisitor::fSign == "C" && clefvisitor::fLine == 4 && clefvisitor::fOctaveChange == 0) {
        key = "tenor";
    }else{
        key = "unknown";
    }
    clefsInPart[currentPart].insert(key);
}

int  xml2guidovisitor::getTransposeInstrumentChromatic () {
    // The chromatic element, representing the number of chromatic steps to add to the written pitch, is the one required element. The diatonic, octave-change, and double elements are optional elements.
    return fChromatic + (transposevisitor::fOctaveChange * 12);
}

std::string xml2guidovisitor::getTransposeInstrumentName() {
    switch (fChromatic) {
        case -2:
            return "Bb";
            break;
            
        case -3:
            return "A";
            break;
            
        case -5:
            return "G";
            break;
            
        case 3:
            return "Eb";
            break;
            
        case 2:
            return "D";
            break;
            
        default:
            return "C";
            break;
    }
}

int xml2guidovisitor::getStavesForFirstPart() {
    return stavesInPart.begin()->second;
}

std::vector<std::string> xml2guidovisitor::getAllClefsOfFirstPart() {
    std::set<string> clefsSet = clefsInPart.begin()->second;
    std::vector<string> clefsVector(clefsSet.begin(), clefsSet.end());
    return clefsVector;
}


int xml2guidovisitor::getTotalStaves() {
    int totalStaves = 0;
    for (auto&& e : stavesInPart) {
        totalStaves += e.second;
    }
    return totalStaves;
}

int xml2guidovisitor::getTotalMeasures() {
    return fTotalMeasures;
}

std::map<double, int> xml2guidovisitor::getMeasureMap() {
    return measurePositionMap;
}

std::map<double, std::pair<int, int>> xml2guidovisitor::getTimeSignatureMap() {
    return timeSignatureMap;
}
    
double xml2guidovisitor::getTotalDuration() {
    return fTotalDuration;
}

int xml2guidovisitor::getPartsAvailable() {
    return fPartsAvailable;
}
}

