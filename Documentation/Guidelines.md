Project Guidelines

All tests are to be done using the Unity C-testing framework

Documentation including in this file and the README.md are to be kept up-to-date as decisions are made and changed.

All tests should be runnable via a cmake command (do not use the Unity Ruby test runner)

All tests are to be written in C

Useful high-level testing operations should be maintained in a layer and used to keep tests high-level and maintainable. This code should be kept in the Common directory. Eventually common modules may be used for testing other Model T software

All tests are to execute on a virtual PC-8201a via the socket (telnet) feature of Virtual T, an emulator for the TRS-80 Model 100 family of (vintage) laptops. Also referred to collectively as the Model T.

Jargon unique to this project should be maintained in the Documentation/Glossary.md file.