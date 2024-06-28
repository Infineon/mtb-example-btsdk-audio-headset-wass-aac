
This folder contains examples of voice Prompt messages (English/USA language in the en-us folder).

These audio files were created from the https://text-to-speech-demo.ng.bluemix.net/ web site.
The voice selected is: American English (en-US): Allison (female, expressive, transformable)
For real application, it is recommended to record real voice message with a good microphone.

The voice message must be:
  - recorded in .wav format (8kHz, 16 bits per samples, little-endian, mono)
  - placed in <language>/wav folder
  - prefixed with the voice number [0..xx]. This allows the makefile to sort them.

Several languages folders (e.g. en-us, en-uk, zh, zh-hk, etc.).

The makefile will:
  - process (compress using the Voice Prompt Codec Encoder) every language folder.
  - create a Voice Prompt File System (VPFS) containing VPC binary (vpfs\_vpc.bin) for every language.

Note that the Voice Compression requires a lot of processing time (around 30 seconds to Compress 1
second of audio).
The Voice Prompt Codec does not compress correctly basic sinewaves (e.g. tone). It is optimized for Voice.
The vpc\_decoder.exe application can be used to check result of the Codec. The output file containbs raw
PCM samples (8kHz, 16 bits per samples, little-endian, mono).

To be able to play Voice Prompt messages:
  - the application must b ecompiled with the VOICE\_PROMPT=1 option.
  - one VPFS file must be downloaded in the board.

The download-vpfs.sh script can be used to download a VPFS (e.g. en-us/vpfs\_vps.bin) in Flash.
The location where the VPFS file is downloaded is application specific (depend on memory mapping).
The same VPFS file must be downloaded in both Primary and Secondary devices.
download-vpfs.sh usage example:
$./download-vpfs.sh COM4 ../voice\_prompt/en-us/vpfs\_vpc.bin d6000

Using a different Voice Prompt File System file per language allows manufacturers to download a new
language to adapt to customer's locations (e.g. USA, China, etc.).
Downloading VPFS file Over The Air must be implemented by manufacturers (not supported by the sample application).
