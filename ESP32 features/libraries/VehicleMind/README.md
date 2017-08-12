# Vehicle Mind Library


## Packeges and Classes

+ ChipID

+ Network Manager

    + MQTT
    + TCP
    + SNTP
    + Timer

+ Sensors

    + GPS
    + MEMS
    + OBD

+ Tools

    + Binary Packeger
    + UUID
    + Verbose


## Structure

According to the platformio's library dependency finder (ldf) ver. 1.5 all the header files which are needed to be included in main program should be placed in the src directory's root of the library folder. All other header files and source files can be placed in any ordering and place under the src directory.

Because of using some specific header files we put those ones in a general header file named VehicleMind.h and put it on the root. The nested structure for other classes by this header file is preserved.

Due to dependency of VehicleMind library on FreematicsOne library (Sensors classes), we had to change the ldf's mode from chain (default) to deep. Diffewrence between these two is based on the parsing part of the compile process. Deep mode in addition to source files of the project, also, parses all source files of the each found dependency recursively. (\*)

For More Details, please have a look at VehicleMind.h, library.properties, and platformio.ini files.

---

\* In order to make the library more standard this dependency should be resolved. One possible way for reaching that is to merge both FreematicsOne and VehicleMind libraries in to one standalone one.

