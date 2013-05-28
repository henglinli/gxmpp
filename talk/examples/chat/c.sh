clang++ *.cc ../../xmpp/*.o ../../xmllite/*.o ../../base/*.o  -I ../../../ -DPOSIX -DLINUX  -lexpat -lssl -pthread -lrt
