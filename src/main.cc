#include "app/cart.hh"


auto main(int argc, char **argv) -> int
{ return cart::app::cart::run({ argv, std::size_t(argc) }); }
