#include <iostream>
#include <string>
#include <fstream>



#include "json_reader.h"
#include "transport_catalogue.h"


using namespace std;

int main() {
   
   transport_catalogue::processing::TransportCatalogue catalogue;
   fstream file("C:\\Users\\1655082\\Desktop\\testdata\\in.txt");
   
   JsonReader jreader;
   jreader.JsonIn(catalogue, file);
  
//   MapRenderer maprend = jreader.GetMapData();
  // maprend.SetTransportCatalogue(catalogue);
  //maprend.CreateSvg(std::cout);
   //maprend.C
  // maprend.GetStatFromJson(jreader.data_request_);


 
   ofstream ofile("C:\\Users\\1655082\\Desktop\\testdata\\out.txt");
    
  //statreader::StatReader sreader(&ifile, &file);
  // sreader.StartRequestParcing(catalogue);
   jreader.JsonOut(catalogue, ofile);
}