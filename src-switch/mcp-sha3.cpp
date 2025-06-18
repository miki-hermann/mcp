/**************************************************************************
 *                                                                        *
 *                                                                        *
 *	         Multiple Classification Project (MCP)                    *
 *                                                                        *
 *	Author:   Miki Hermann                                            *
 *	e-mail:   hermann@lix.polytechnique.fr                            *
 *	Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France        *
 *                                                                        *
 *	Author: Gernot Salzer                                             *
 *	e-mail: gernot.salzer@tuwien.ac.at                                *
 *	Address: Technische Universitaet Wien, Vienna, Austria            *
 *                                                                        *
 *	Version: all                                                      *
 *      File:    mcp-switch.cpp                                           *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 * Computes the SHA3 hash of a (binary) file                              *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

// compile with
// g++ -O4 -o mcp-switch mcp-switch.cpp -lssl -lcrypto

#include <iostream>
#include <vector>
#include <sstream>		// for ostringstream
#include <iomanip>		// for setw, hex, and setfill
#include <openssl/evp.h>	// for all other OpenSSL function calls
#include <openssl/sha.h>	// for SHA512_DIGEST_LENGTH
#include <fstream>
#include <string>
#include <cstdint>

using namespace std;

const string whereis_file("/tmp/mcp-whereis.txt");

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// helper function to print the digest bytes as a hex string
string bytes_to_hex_string(const vector<uint8_t> &bytes) {
  ostringstream stream;
  for (uint8_t b : bytes)
    stream << setw(2) << setfill('0') << hex << static_cast<int>(b);
  return stream.str();
}

// perform the SHA3-512 hash
string sha3_512(const string &input) {
  uint32_t digest_length = SHA512_DIGEST_LENGTH;
  const EVP_MD *algorithm = EVP_sha3_512();
  uint8_t *digest = static_cast<uint8_t*>(OPENSSL_malloc(digest_length));
  EVP_MD_CTX *context = EVP_MD_CTX_new();
  EVP_DigestInit_ex(context, algorithm, nullptr);
  EVP_DigestUpdate(context, input.c_str(), input.size());
  EVP_DigestFinal_ex(context, digest, &digest_length);
  EVP_MD_CTX_destroy(context);
  string output = bytes_to_hex_string(vector<uint8_t>(digest, digest + digest_length));
  OPENSSL_free(digest);
  return output;
}

string sha3_file (const string &filename) {
  ifstream infile(filename, ifstream::binary);
  if (!infile)
    return "";
  streambuf *pbuf = infile.rdbuf();
  const size_t size = pbuf->pubseekoff(0,infile.end);
  pbuf->pubseekpos(0,infile.in);
  char *buffer = new char[size+1];
  pbuf->sgetn(buffer, size);
  const string output = bytes_to_hex_string(vector<uint8_t>(buffer, buffer + size));
  const string sha3_output = sha3_512(output);
  infile.close();
  delete[] buffer;
  return sha3_output;
}

size_t search_file (const string &mv_file, string &whis) {
  const string whereis("whereis " + mv_file + " > " + whereis_file);
  system(whereis.c_str());
  ifstream whereis_stream;
  whereis_stream.open(whereis_file);
  getline(whereis_stream, whis);
  whereis_stream.close();
  remove(whereis_file.c_str());
  size_t pos = whis.rfind(" ");
  whis = pos == string::npos ? whis.erase(whis.length()-1) : whis.erase(0,pos+1);
  return pos;
}

//////////////////////////////////////////////////////////////////////////////
