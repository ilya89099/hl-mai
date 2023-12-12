#include "http_order_server.h"

int main(int argc, char* argv[]) {
  HTTPOrderServer app;
  return app.run(argc, argv);
}
