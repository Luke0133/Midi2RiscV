#include "MidiFile.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>

// This code uses the source code from the library MidiFIle, from Craig Stuart Sapp https://github.com/craigsapp/midifile
// Further liscencing can be seen at the end of the code

typedef std::ostringstream ostringstream;
typedef std::ofstream ofstream;
typedef std::string string;
using namespace std;
using namespace smf;

class Note{
private:
    float Duration;
    float Beginning;
    int Pitch;

public:
    float getDuration(void){return Duration;}
    float getBeginning(void){return Beginning;}
    int getPitch(void){return Pitch;}
    void setDuration(float Duration){
        this->Duration = Duration;
        return;
        }
    void setBeginning(float Beginning){
        this->Beginning = Beginning;
        return;
        }
    void setPitch(int Pitch){
        this->Pitch = Pitch;
        return;
        }
};

typedef std::vector<Note> note_vector;
int main(int argc, char** argv) {
    // Options options;
    // options.process(argc, argv);
    MidiFile midifile;
    if (argc <= 1) {
        cout << "Usage: Midi2RiscV.exe filename.mid(required) outputname(optional, no file extension)\n";
        return 1;
    }
    else {
        midifile.read(argv[1]);
        if (!midifile.read(argv[1])) {
            cerr << "Failed to read MIDI file.\n";
            return 1;
        }
    }

    // Analysing midi file
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();

    // Getting output name
    string outputName = string(argv[1]);

    if (argc != 3){
        outputName = string(argv[1]);
        for (size_t i = outputName.length() - 1; i >= 0; i--){
            if (outputName[i] == '.'){
                outputName = outputName.substr(0,i);
                break;
            }
        }
    }
    else{
        outputName = string(argv[2]);
    }

    // Naming output file
    ofstream output(outputName + ".data", ofstream::out);
    if (!output.is_open()) {
        cerr << "Failed to open output file." << endl;
        return 1;
    }

    ostringstream outputText;

    int tracks = midifile.getTrackCount();

    for (int track=0; track<tracks; track++) {  
        std::vector<note_vector> currentTrack;     // Creates vector of note vectors, i.e., a track that can store voices of notes 

        for (int event=0; event < midifile[track].size(); event++) {
            if (midifile[track][event].isNoteOn()){
                // If it's the beginning of a note
                if ((int)midifile[track][event][2] != 0){  // and the velocity isn't 0
                    Note note;   // Creates new note

                    note.setBeginning(midifile[track][event].seconds*1000);    // Sets note's beginning

                    note.setDuration(midifile[track][event].getDurationInSeconds()*1000); // and its duration

                    note.setPitch((int)midifile[track][event][1]);

                    bool validPlacement = false;  // if you can store on any available voice list
                    int tracksize = (int)currentTrack.size();
                    for (int i=0; i < tracksize; i++){
                        if (validPlacement) break;
                        if (currentTrack[i].size() == 0){
                            if ((note.getBeginning() != 0) && (note.getBeginning() >= 1)){
                                Note rest;
                                rest.setBeginning(0);
                                rest.setDuration(note.getBeginning());
                                rest.setPitch(0);
                                currentTrack[i].push_back(rest);
                            }
                            currentTrack[i].push_back(note);
                            validPlacement = true;
                        }
                        else {
                            if (note.getBeginning() != currentTrack[i].back().getBeginning()){
                                if ((int)note.getBeginning() >= (int)(currentTrack[i].back().getBeginning() + currentTrack[i].back().getDuration())){
                                    int compare = (int)note.getBeginning() - (int)(currentTrack[i].back().getBeginning() + currentTrack[i].back().getDuration());
                                    if ((compare != 0) && (compare >= 1)){
                                        Note rest;
                                        rest.setBeginning(currentTrack[i].back().getBeginning() + currentTrack[i].back().getDuration());
                                        rest.setDuration(compare);
                                        rest.setPitch(0);
                                        currentTrack[i].push_back(rest);
                                    }
                                    currentTrack[i].push_back(note);
                                    validPlacement = true;
                                } else {
                                    validPlacement = false;
                                }
                            }
                            else {
                                validPlacement = false;
                            }
                        }
                    }

                    if (!validPlacement){   // There are no available voices to put new note into, so create a new voice
                        note_vector NewVoice;                        // Gets a vector for a new voice (a vector of notes)
                        // And stores new note
                        if ((note.getBeginning() != 0) && (note.getBeginning() >= 1)){
                            Note rest;
                            rest.setBeginning(0);
                            rest.setDuration(note.getBeginning());
                            rest.setPitch(0);
                            NewVoice.push_back(rest);
                        }
                        NewVoice.push_back(note);
                        currentTrack.push_back(NewVoice);            // Stores NewVoice in track vector
                    }
                }
              //  cout << endl;
            }
        }

        for (int i = 0; i < currentTrack.size(); i++){   // looping through voices of current track
            int counter = 0;
            if (currentTrack[i].size() != 0){
                outputText << outputName << "_Track" << std::to_string(track) << "_Voice" << std::to_string(i) << "_SIZE: .word ";
                ostringstream append;
                for (int n = 0; n < currentTrack[i].size(); n++){  // Looping through notes of current track
                    append << currentTrack[i][n].getPitch() << "," << static_cast<int>(currentTrack[i][n].getDuration()) << ",";
                    counter++;
                }
                outputText << counter << ",\n" << outputName << "_Track" << track << "_Voice" << i << "_NOTES: " << append.str() << "\n\n";    
            }
        }
    }
    cout << "Done! Output file named " << outputName << ".data was created.\n" ;
    output << outputText.str();
    output.close();
    return 0;
}



/*
Copyright (c) 1999-2018, Craig Stuart Sapp
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   and the following disclaimer in the documentation and/or other materials 
   provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/