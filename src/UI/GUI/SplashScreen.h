#ifndef SPLASHSCREEN_h
#define SPLASHSCREEN_h

#include "DisplayItem.h"

class SplashScreen : public DisplayItem{

    public:
    SplashScreen();

    void draw() override;

    private:
    //Simple cube :)
    /*double vertex[8][3] = {{1,1,-1},{1,-1,-1},{1,1,1},{1,-1,1},{-1,1,-1},{-1,-1,-1},{-1,1,1},{-1,-1,1}};
    //Faces index in .obj files start at 1, remember that!!
    uint16_t faces[6][4] = {{1,5,7,3},{4,3,7,8},{8,7,5,6},{6,2,4,8},{2,1,3,4},{6,5,1,2}};
    double normals[6][3] = {{0,1,0},{0,0,1},{-1,0,0},{0,-1,0},{1,0,0},{0,0,-1}};*/

    //Simple house :)))))
const double vertex[42][3] PROGMEM = {{0.0,-1.348131,0.0},{0.975517,-0.60291,0.708746},{-0.372607,-0.602911,1.146786},{-1.205804,-0.602905,0.0},{-0.372607,-0.602911,-1.146786},{0.975517,-0.60291,-0.708746},{0.372607,0.602911,1.146786},{-0.975517,0.60291,0.708746},{-0.975517,0.60291,-0.708746},{0.372607,0.602911,-1.146786},{1.205804,0.602905,0.0},{0.0,1.348131,0.0},{-0.219011,-1.146793,0.674059},{0.573391,-1.146793,0.416588},{0.354382,-0.708763,1.090654},{1.146785,-0.708761,0.0},{0.573391,-1.146793,-0.416588},{-0.708753,-1.14679,0.0},{-0.927769,-0.708761,0.674061},{-0.219011,-1.146793,-0.674059},{-0.927769,-0.708761,-0.674061},{0.354382,-0.708763,-1.090654},{1.28215,0.0,0.416589},{1.28215,0.0,-0.416589},{0.0,0.0,1.348131},{0.792412,0.0,1.09066},{-1.28215,0.0,0.416589},{-0.792412,0.0,1.09066},{-0.792412,0.0,-1.09066},{-1.28215,0.0,-0.416589},{0.792412,0.0,-1.09066},{0.0,0.0,-1.348131},{0.927769,0.708761,0.674061},{-0.354382,0.708763,1.090654},{-1.146785,0.708761,0.0},{-0.354382,0.708763,-1.090654},{0.927769,0.708761,-0.674061},{0.219011,1.146793,0.674059},{0.708753,1.14679,0.0},{-0.573391,1.146793,0.416588},{-0.573391,1.146793,-0.416588},{0.219011,1.146793,-0.674059}}; 
const double normals[80][3] PROGMEM = {{0.1024,-0.9435,0.3151},{0.7002,-0.6617,0.268},{-0.268,-0.9435,0.1947},{-0.268,-0.9435,-0.1947},{0.1024,-0.9435,-0.3151},{0.905,-0.3304,0.268},{0.0247,-0.3304,0.9435},{-0.8897,-0.3304,0.3151},{-0.5746,-0.3304,-0.7488},{0.5346,-0.3304,-0.7779},{0.8026,-0.1256,0.5831},{-0.3066,-0.1256,0.9435},{-0.9921,-0.1256,0.0},{-0.3066,-0.1256,-0.9435},{0.8026,-0.1256,-0.5831},{0.4089,0.6617,0.6284},{-0.4713,0.6617,0.5831},{-0.7002,0.6617,-0.268},{0.0385,0.6617,-0.7488},{0.724,0.6617,-0.1947},{0.268,0.9435,-0.1947},{0.4911,0.7947,-0.3568},{0.4089,0.6617,-0.6284},{-0.1024,0.9435,-0.3151},{-0.1876,0.7947,-0.5773},{-0.4713,0.6617,-0.5831},{-0.3313,0.9435,0.0},{-0.6071,0.7947,0.0},{-0.7002,0.6617,0.268},{-0.1024,0.9435,0.3151},{-0.1876,0.7947,0.5773},{0.0385,0.6617,0.7488},{0.268,0.9435,0.1947},{0.4911,0.7947,0.3568},{0.724,0.6617,0.1947},{0.8897,0.3304,-0.3151},{0.7947,0.1876,-0.5773},{0.5746,0.3304,-0.7488},{-0.0247,0.3304,-0.9435},{-0.3035,0.1876,-0.9342},{-0.5346,0.3304,-0.7779},{-0.905,0.3304,-0.268},{-0.9822,0.1876,0.0},{-0.905,0.3304,0.268},{-0.5346,0.3304,0.7779},{-0.3035,0.1876,0.9342},{-0.0247,0.3304,0.9435},{0.5746,0.3304,0.7488},{0.7947,0.1876,0.5773},{0.8897,0.3304,0.3151},{0.3066,0.1256,-0.9435},{0.3035,-0.1876,-0.9342},{0.0247,-0.3304,-0.9435},{-0.8026,0.1256,-0.5831},{-0.7947,-0.1876,-0.5773},{-0.8897,-0.3304,-0.3151},{-0.8026,0.1256,0.5831},{-0.7947,-0.1876,0.5773},{-0.5746,-0.3304,0.7488},{0.3066,0.1256,0.9435},{0.3035,-0.1876,0.9342},{0.5346,-0.3304,0.7779},{0.9921,0.1256,0.0},{0.9822,-0.1876,0.0},{0.905,-0.3304,-0.268},{0.4713,-0.6617,-0.5831},{0.1876,-0.7947,-0.5773},{-0.0385,-0.6617,-0.7488},{-0.4089,-0.6617,-0.6284},{-0.4911,-0.7947,-0.3568},{-0.724,-0.6617,-0.1947},{-0.724,-0.6617,0.1947},{-0.4911,-0.7947,0.3568},{-0.4089,-0.6617,0.6284},{0.7002,-0.6617,-0.268},{0.6071,-0.7947,0.0},{0.3313,-0.9435,0.0},{-0.0385,-0.6617,0.7488},{0.1876,-0.7947,0.5773},{0.4713,-0.6617,0.5831}}; 
const uint8_t faces[80][3] PROGMEM = {{1,14,13},{2,14,16},{1,13,18},{1,18,20},{1,20,17},{2,16,23},{3,15,25},{4,19,27},{5,21,29},{6,22,31},{2,23,26},{3,25,28},{4,27,30},{5,29,32},{6,31,24},{7,33,38},{8,34,40},{9,35,41},{10,36,42},{11,37,39},{39,42,12},{39,37,42},{37,10,42},{42,41,12},{42,36,41},{36,9,41},{41,40,12},{41,35,40},{35,8,40},{40,38,12},{40,34,38},{34,7,38},{38,39,12},{38,33,39},{33,11,39},{24,37,11},{24,31,37},{31,10,37},{32,36,10},{32,29,36},{29,9,36},{30,35,9},{30,27,35},{27,8,35},{28,34,8},{28,25,34},{25,7,34},{26,33,7},{26,23,33},{23,11,33},{31,32,10},{31,22,32},{22,5,32},{29,30,9},{29,21,30},{21,4,30},{27,28,8},{27,19,28},{19,3,28},{25,26,7},{25,15,26},{15,2,26},{23,24,11},{23,16,24},{16,6,24},{17,22,6},{17,20,22},{20,5,22},{20,21,5},{20,18,21},{18,4,21},{18,19,4},{18,13,19},{13,3,19},{16,17,6},{16,14,17},{14,1,17},{13,15,3},{13,14,15},{14,2,15}};
const uint8_t normalIndexes[80] PROGMEM = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80}; 
std::vector<std::array<uint16_t, 2>> edges;

    volatile bool fillPolygons = true;
    const double angularVelocity = PI/4.0;
    const double transitionAngularVelocity = HALF_PI;
    volatile uint8_t color[3] = {200,200,200};
    volatile uint16_t backColor = TFT_BLACK;

    //Camera plane height and width
    static const uint16_t w = 160, h = 128;
    //Remember, this vector must be normalized!
    const double sun[3] = {0.5774,-0.5774,-0.5774};

    volatile double inputVariable = 0;

    uint8_t getEdgesCount(uint8_t index);

    void startParameters(TFT_eSprite*);
    void smoothRotation(double th, double &omega);
    void multMatrix(double in[4], double matrix[4][4], double out[4]);
    void multMatrix(const double in[4], double matrix[4][4], double out[4]);
    void multMatrix(double matrixA[4][4], double matrixB[4][4], double outMatrix[4][4]);
    void multMatrixHomogeneous(double in[4], double matrix[4][4], double out[4]);

};

#endif