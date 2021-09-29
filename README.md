# lopass
A real time audio processing plugin in the Apple format, AudioUnit, version 2 API using the CoreAudio C Utility classes.
All code is written in C++.

The AudioUnit can be hosted in Logic X, and is currently working with Logic 10.6.3, running on macOS 11.5.2 (Big Sur). This version does not implement a custom CocoaUI object currently, a generic GUI will be instaniated from Logic, providing access to the parameters controls.

Parameter controls include;
 - Cutoff Frequency, in hertz.
 - Resonance, in decibels.

To install, download the project files and build the target executable. Copy the derived .component folder into /Library/Audio/Plug-Ins/Components/, restart the machine. Load Logic to scan for the new PlugIn, it should appear under the folder 'DAVE' as 'LoPass Filter'.

<img width="1920" alt="LoPass Filter screenshot" src="https://user-images.githubusercontent.com/67363039/135182936-8bc9bb12-7e61-4097-9806-bdd4dfd976cb.png">
