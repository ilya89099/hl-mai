#include "http_service_server.h"

int main(int argc, char* argv[]) {
  HTTPServiceServer app;
  return app.run(argc, argv);
}
