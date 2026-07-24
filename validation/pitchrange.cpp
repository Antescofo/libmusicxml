/*
 Focused validation for xml2guidovisitor's selected-part written pitch range.
*/

#include <cmath>
#include <cstdlib>
#include <iostream>

#include "xml.h"
#include "xml2guidovisitor.h"
#include "xmlfile.h"
#include "xmlreader.h"

using namespace MusicXML2;

namespace {

const char* kScore = R"musicxml(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE score-partwise PUBLIC "-//Recordare//DTD MusicXML 3.1 Partwise//EN"
  "http://www.musicxml.org/dtds/partwise.dtd">
<score-partwise version="3.1">
  <part-list>
    <score-part id="P1"><part-name>First</part-name></score-part>
    <score-part id="P2"><part-name>Second</part-name></score-part>
    <score-part id="P3"><part-name>No pitched notes</part-name></score-part>
    <score-part id="P4"><part-name>Middle C</part-name></score-part>
  </part-list>
  <part id="P1">
    <measure number="1">
      <attributes>
        <divisions>4</divisions>
        <staves>2</staves>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef number="1"><sign>G</sign><line>2</line></clef>
        <clef number="2"><sign>F</sign><line>4</line></clef>
      </attributes>
      <note>
        <pitch><step>C</step><octave>4</octave></pitch>
        <duration>4</duration><voice>1</voice><staff>1</staff>
      </note>
      <note>
        <chord/>
        <pitch><step>B</step><alter>0.5</alter><octave>5</octave></pitch>
        <duration>4</duration><voice>1</voice><staff>1</staff>
      </note>
      <note>
        <grace/>
        <pitch><step>A</step><octave>2</octave></pitch>
        <voice>1</voice><staff>1</staff>
      </note>
      <note>
        <cue/>
        <pitch><step>C</step><octave>8</octave></pitch>
        <duration>4</duration><voice>1</voice><staff>1</staff>
      </note>
      <note>
        <rest/><duration>4</duration><voice>1</voice><staff>1</staff>
      </note>
      <note>
        <unpitched><display-step>C</display-step><display-octave>9</display-octave></unpitched>
        <duration>4</duration><voice>1</voice><staff>1</staff>
      </note>
      <backup><duration>16</duration></backup>
      <note>
        <pitch><step>G</step><octave>5</octave></pitch>
        <duration>4</duration><voice>2</voice><staff>2</staff>
      </note>
    </measure>
  </part>
  <part id="P2">
    <measure number="1">
      <attributes>
        <divisions>4</divisions>
        <time><beats>4</beats><beat-type>4</beat-type></time>
        <clef><sign>G</sign><line>2</line></clef>
      </attributes>
      <note>
        <pitch><step>D</step><octave>3</octave></pitch>
        <duration>4</duration><voice>1</voice>
      </note>
    </measure>
    <measure number="2">
      <note print-object="no">
        <pitch><step>F</step><alter>-0.5</alter><octave>6</octave></pitch>
        <duration>4</duration><voice>1</voice>
      </note>
    </measure>
  </part>
  <part id="P3">
    <measure number="1">
      <attributes><divisions>4</divisions></attributes>
      <note><rest/><duration>4</duration><voice>1</voice></note>
      <note>
        <cue/><pitch><step>C</step><octave>8</octave></pitch>
        <duration>4</duration><voice>1</voice>
      </note>
      <note>
        <unpitched><display-step>C</display-step><display-octave>4</display-octave></unpitched>
        <duration>4</duration><voice>1</voice>
      </note>
    </measure>
  </part>
  <part id="P4">
    <measure number="1">
      <attributes><divisions>4</divisions></attributes>
      <note>
        <pitch><step>C</step><octave>4</octave></pitch>
        <duration>4</duration><voice>1</voice>
      </note>
    </measure>
  </part>
</score-partwise>)musicxml";

bool closeTo(double lhs, double rhs)
{
    return std::fabs(lhs - rhs) < 0.000001;
}

bool expectRange(int renderedPartNumber, int pitchRangePartNumber,
                 double expectedMinimum, double expectedMaximum,
                 int beginMeasure = 0, int endMeasure = 0)
{
    xmlreader reader;
    SXMLFile file = reader.readbuff(kScore);
    if (!file || !file->elements()) {
        std::cerr << "Could not parse range validation score" << std::endl;
        return false;
    }

    const int endMeasureOffset = endMeasure > 0 ? 1 : 0;
    xml2guidovisitor visitor(true, true, true, renderedPartNumber,
                            beginMeasure, 0.0,
                            endMeasure, endMeasureOffset, 0.0,
                            pitchRangePartNumber);
    visitor.convert(file->elements());

    if (!visitor.hasSoloWrittenPitchRange()) {
        std::cerr << "Expected a range for part " << pitchRangePartNumber << std::endl;
        return false;
    }
    if (!closeTo(visitor.getSoloWrittenPitchRangeMin(), expectedMinimum)
        || !closeTo(visitor.getSoloWrittenPitchRangeMax(), expectedMaximum)) {
        std::cerr << "Unexpected range for part " << pitchRangePartNumber << ": "
                  << visitor.getSoloWrittenPitchRangeMin() << "..."
                  << visitor.getSoloWrittenPitchRangeMax() << std::endl;
        return false;
    }
    return true;
}

bool expectNoRange(int renderedPartNumber, int pitchRangePartNumber)
{
    xmlreader reader;
    SXMLFile file = reader.readbuff(kScore);
    xml2guidovisitor visitor(true, true, true, renderedPartNumber,
                            0, 0.0, 0, 0, 0.0,
                            pitchRangePartNumber);
    visitor.convert(file->elements());
    if (visitor.hasSoloWrittenPitchRange()) {
        std::cerr << "Expected no range for part " << pitchRangePartNumber << std::endl;
        return false;
    }
    return true;
}

bool expectDefaultRangeSelection(int renderedPartNumber,
                                 double expectedMinimum, double expectedMaximum)
{
    xmlreader reader;
    SXMLFile file = reader.readbuff(kScore);
    xml2guidovisitor visitor(true, true, true, renderedPartNumber);
    visitor.convert(file->elements());

    return visitor.hasSoloWrittenPitchRange()
        && closeTo(visitor.getSoloWrittenPitchRangeMin(), expectedMinimum)
        && closeTo(visitor.getSoloWrittenPitchRangeMax(), expectedMaximum);
}

bool expectRenderingUnaffected(int renderedPartNumber,
                               int firstPitchRangePartNumber,
                               int secondPitchRangePartNumber)
{
    xmlreader firstReader;
    SXMLFile firstFile = firstReader.readbuff(kScore);
    xml2guidovisitor firstVisitor(true, true, true, renderedPartNumber,
                                 0, 0.0, 0, 0, 0.0,
                                 firstPitchRangePartNumber);
    const std::string firstRendering = firstVisitor.convertToString(firstFile->elements());

    xmlreader secondReader;
    SXMLFile secondFile = secondReader.readbuff(kScore);
    xml2guidovisitor secondVisitor(true, true, true, renderedPartNumber,
                                  0, 0.0, 0, 0, 0.0,
                                  secondPitchRangePartNumber);
    const std::string secondRendering = secondVisitor.convertToString(secondFile->elements());

    if (firstRendering != secondRendering) {
        std::cerr << "Pitch-range selection changed rendering for part "
                  << renderedPartNumber << std::endl;
        return false;
    }
    return true;
}

} // namespace

int main()
{
    // Existing callers retain their previous selected-part/first-part behavior.
    if (!expectDefaultRangeSelection(0, 45.0, 83.5)) return EXIT_FAILURE;
    if (!expectDefaultRangeSelection(2, 50.0, 88.5)) return EXIT_FAILURE;

    // partNumber 0 converts every part and explicit range part 0 selects P1.
    if (!expectRange(0, 0, 45.0, 83.5)) return EXIT_FAILURE;

    // Explicit selection reports P2 and retains fractional alterations.
    if (!expectRange(2, 2, 50.0, 88.5)) return EXIT_FAILURE;

    // Rendering every part can measure only the independently selected solo.
    if (!expectRange(0, 2, 50.0, 88.5)) return EXIT_FAILURE;

    // A non-rendered part can supply the range without entering the output.
    if (!expectRange(1, 2, 50.0, 88.5)) return EXIT_FAILURE;

    // Range calculation covers the complete part despite excerpt rendering.
    if (!expectRange(2, 2, 50.0, 88.5, 1, 1)) return EXIT_FAILURE;

    // Selecting a range source never changes all-parts or single-part output.
    if (!expectRenderingUnaffected(0, 1, 2)) return EXIT_FAILURE;
    if (!expectRenderingUnaffected(1, 1, 2)) return EXIT_FAILURE;

    // A part with only cue/rest/unpitched notes and a missing part are invalid.
    if (!expectNoRange(0, 3)) return EXIT_FAILURE;

    // MusicXML C4 is conventional MIDI 60 at the API boundary.
    if (!expectRange(0, 4, 60.0, 60.0)) return EXIT_FAILURE;

    if (!expectNoRange(0, 5)) return EXIT_FAILURE;

    std::cout << "pitch range validation passed" << std::endl;
    return EXIT_SUCCESS;
}
