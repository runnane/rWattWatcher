#include <wiringPi.h>
#include <iostream>
#include <ctime>
#include <fstream>
#include <cstdlib>

using namespace std;

void reportUsage(int, float, float, float,int);
void reportUsageCurrent(float watt);

int main() {
  unsigned long long Int64 = 0;
  clock_t prevchange = clock();
  clock_t last_report = clock();
  clock_t thischange;
  int blinks_since_last = 0;
  int first=1;
  int prev_status = 0;
  int cur_status = 0;
  int counter = 0;

  float avgkwh;
  float avgrate;
  float avgbps;

  float correction=0.85f;

  if (wiringPiSetup() == -1)
    return 1 ;

  pinMode(3, INPUT);

  while(1) {
    cur_status=digitalRead(3);
    if(cur_status == 1 && prev_status == 0){
        thischange=clock();
        float tsl = ((float)thischange-prevchange)/CLOCKS_PER_SEC;
        if(tsl < 0.05){
                // should not be, prob reading error;
                prev_status = cur_status;
                prevchange=thischange;
                continue;
        }
        float bps = 1.0f/tsl;
        prev_status = cur_status;
        counter++;
        blinks_since_last++;
        float kwh = 3600/tsl/1000.0f;
        cout << "[+] COUNT  (" << counter << " blinks) - " << "(" << tsl  << " sec) - " << "(" << bps  << " bps) - " << "(" << kwh << " kWh)" << endl;
        avgrate+=tsl;
        avgbps+=bps;
        avgkwh+=kwh;
        // live usage
        reportUsageCurrent((3600/tsl)*correction);

        // over-time-report
        if(thischange-last_report > 30000000){ //each 30 sec (120W w/ 10.000x pr kWh )
          reportUsage(counter,(avgrate/blinks_since_last),(avgbps/blinks_since_last),(avgkwh/blinks_since_last),blinks_since_last);
          last_report=thischange;
          blinks_since_last=0;
          avgkwh=avgrate=avgbps=0.0f;
        }
        prevchange=thischange;
    }else{
        prev_status = cur_status;
    }
    //usleep(10);
  }

  return 0 ;
}

void reportUsageCurrent(float watt){
        ofstream myfile;
        myfile.open("/var/smarthouse/currentWatt");
        myfile << watt << ";W;";
        myfile.close();
}

void reportUsage(int counter, float tsl, float bps, float avg, int watt){
        cout << "[i] REPORT (" << counter << " blinks) - " << "(" << tsl  << " sec) - " << "(" << bps  << " bps) - " << "(" << avg << " kWh) - " << "(" << watt << " Wh)"<< endl;
        ofstream myfile;
        myfile.open("/var/smarthouse/wattusage");
        myfile << counter << ";" << tsl << ";" << bps << ";" << avg << ";" << watt;
        myfile.close();

        // increment wattcounter
        string line;
        ifstream myfile2;
        myfile2.open("/var/smarthouse/wattcounter");
        getline(myfile2,line);
        long value = atol(line.c_str());
        myfile.close();


        myfile.open("/var/smarthouse/wattcounter");
        myfile << value+watt;
        myfile.close();

        // update lastupdated
        //myfile.open("/var/smarthouse/lastupdate");
        //myfile << counter;
        //myfile.close();


        return;
}
