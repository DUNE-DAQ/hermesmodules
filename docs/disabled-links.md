# Configure both links, acquire from one

DAPHNE firmware link configuration and DAQ participation are separate.
Declare both HermesDataSender objects in the NetworkDetectorToDaqConnection,
with distinct link_id values and their detector streams. Disable the disconnected
sender in the Session.disabled relation.

DaphneApplication in appmodel retains disabled Hermes senders for an active
control_host when generating the HermesModule. It does not add their streams
to active board selection or create controllers for entirely disabled boards.
HermesModule configures UDP addresses and geo IDs for every declared link during
conf, leaves all links disabled, and starts/checks only session-enabled links.

For DAPHNE-015 build 662c3fe the database declares physical links 0 and 1.
Link 0 carries logical streams 0 and 1 (source IDs 800 and 801); link 1 carries
streams 2 and 3 (802 and 803) and remains disabled. No manual links.py step
is required with both the appmodel and hermesmodules changes installed.
