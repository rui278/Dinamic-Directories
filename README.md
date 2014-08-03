Dinamic-Directories
===================

A dinamic directory based chat application

Done for Computer Networks class. 

About:

This project implements a chat application based on multiple dinamic name servers.

All participants connect to a main name server (ddss executable. Built for linux) with a username composed of name.surname

Uppon registry the name server checks if there is any other user with username *.surname

If there is such a user, then the new user will be redirected to said user, with whom the user will register, and will serve
as it's name server.

Any request to speak with user a.b will be sent to the main name server, redirected to n.b - that was the first user with
b as a surname - wich will act as a.b's name server.

Uppon n.b's unsubscription the new name server for the *.b family will be resolved.


By
Rui Albuquerque
Maria Braga

