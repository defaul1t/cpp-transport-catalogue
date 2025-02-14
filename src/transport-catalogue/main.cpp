#include <fstream>
#include <iostream>
#include <string>

#include "json_reader.h"
#include "transport_catalogue.h"

using namespace std;

int main() {
  transport_catalogue::processing::TransportCatalogue catalogue;
  fstream file("../data-samples/in_case1.txt");

  JsonReader jreader;
  jreader.JsonIn(catalogue, file);

  //   MapRenderer maprend = jreader.GetMapData();
  // maprend.SetTransportCatalogue(catalogue);
  // maprend.CreateSvg(std::cout);
  // maprend.C
  // maprend.GetStatFromJson(jreader.data_request_);
  ofstream ofile("../data-samples/out_case1.txt");

  // statreader::StatReader sreader(&ifile, &file);
  //  sreader.StartRequestParcing(catalogue);
  jreader.JsonOut(catalogue, ofile);
}