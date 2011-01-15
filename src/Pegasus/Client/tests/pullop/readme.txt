Pegasus test client to test pull operations.  Executes a single pull
sequence determined by the Type parameter consisting of an open
request and a number of pull requests.

The program options determine
   - The server (default is to the local host)
   - Parameters of the pull operation including class name,
     interoperation timeout, max object count, etc.
   - If the results are to be compared with the corresponding non pull
     operation (ex. e (the openEnumerateInstances, pull, ...) would
     be compared with enumerateInstances on the same class.
   - Level or verbosity of status and error information

The goal of this program is to provide a relatively complete test for
the operation of the DMTF pull operations (client and server) that can
be executed from the command line.

This program does not generally test errors.  There is another client
program that executes a test of Server errors.




