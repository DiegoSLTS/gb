# GB Emulator (untitled / sin título)

Estoy haciendo un emulador de GameBoy desde cero para aprender sobre emulación. Es sólo C++ y SFML 2.5, actualmente enfocado en Windows.

----

I'm making a GameBoy emulator from scrath for learning purposes. It's just C++ and SFML 2.5, currently aiming for Windows.

## Getting Started / Empezando

Dejo estas instrucciones para quien esté interesado/a en ejecutar el proyecto en su PC.

----

These are some instructions for anyone interested in getting the project running in their PC

### Prerequisites / Prerequisitos

Actualmente estoy usando Visual Studio 2017 Community en Windows 10, pero el proceso de build es standard así que debería ser fácil adaptarlo a otras tools como CMake u otro IDE.

Vas a necesitar bajar SFML 2.5 de este link: https://www.sfml-dev.org/download.php

----

I'm currently using Visual Studio 2017 Community on a Windows 10, but the current build process is pretty standard so it's probably easy to setup another build tool like CMake or another IDE.

You'll have to download SFML 2.5 from here: https://www.sfml-dev.org/download.php

### Installing / Instalación

Como estoy trabajando con Visual Studio estoy compartiendo el archivo .sln, pero seguro necesitás configurar los paths de SFML para que apunten a la ubicación en la que lo bajaste (voy a intentar simplificarlo en el futuro).

Para configurar los paths de las bibliotecas e includes de SFML seguí estas instrucciones (en inglés): https://www.sfml-dev.org/tutorials/2.5/start-vc.php

Antes de copiar los .dll tenés que generar el .exe compilado, y los .dll tienen que estar en la misma carpeta para que funcione el emulador.

Se puede cargar una rom desde cualquier ubicación local pasando el path absoluto entre comillas como único argumento, o se puede editar el código en main.cpp y roms.h donde hay varios paths hardcodeados. Esta última opción es solo para hacer pruebas y en el futuro planeo reemplazarla con alguna configuración o con una ventana de selección de rom como cualquier otro emulador.

----

As I'm currently using Visual Studio I'm also sharing my .sln, but you might need to update the SFML paths to point to the one on your disk (I'll try to do something better in the future).

To setup SFML bin and include paths follow this: https://www.sfml-dev.org/tutorials/2.5/start-vc.php

Before copying the .dll files you'll have to generate the .exe, and then copy the .dll files in the same folder for the emulator to work.

You can load a rom from any location from your disk passing the absolute path between quotes as an argument when launching the emulator, or by editing the code in main.cpp and roms.h where a few paths are hardcoded. This last method is just for testing purposes and I plan to replace it at some point with a setting or with a browse file window just like any other emulator.

## Current Features / Funcionalidad actual

* Emulación de la CPU, GPU, control, APU y otros componentes internos
* Soportados los tipos de memory banks: RomOnly, MBC1, MBC2, MBC3 y MBC5
* Soporte para batería (para guardar el progreso) en los memory banks y juegos que lo soportan
* Ejecutar juegos de GameBoy en formato .gb (GameBoy original), .gbc (GameBoy Color) o .zip (comprimidos)
* Especificar un path del rom por línea de comando
* Multiples herramientas para debugear (Tiles, TileMaps, Sprites, Paletas, Estado de CPU y GPU, logs de CPU y GPU, Pausar y Avanzar de a pasos)
* Decompilación estática de la rom (se hace sobre la ROM antes de ejecutar el juego)
* Capturar la pantalla en un .png con la tecla C

----

* Emulation of the CPU, GPU, controller, APU and other internal components
* Support of memory banks: RomOnly, MBC1, MBC2, MBC3 and MBC5
* Support for battery (to save progress) on the MBCs and games that support it
* Run GameBoy games in .gb (original GameBoy), .gbc (GameBoy Color) or .zip (compressed) formats
* Specify the rom path through command line arguments
* Multiple debug tools (Tiles, TileMaps, Sprites, Palettes, CPU and GPU state, CPU and GPU logs, Pause and Step emulation)
* Statically decompile a rom (done with the ROM bytes before running the game)
* Capture the screen as a .png when pressing the C key

## Planned Features / Funcionalidad planeada

* UI para cargar un rom y para la funcionalidad básica
* Emulación de Super Game Boy?
* Decompilación dinámica (los roms pueden escribir y ejecutar código en runtime) - Parcialmente implementado en los logs
* Soporte para Linux (capaz funciona, no probé)
* Grabar videos y audio

----

* UI to load a rom and the basic funtionality
* Emulation of the Super Game Boy?
* Dynamic decompile (roms can write and execute code on runtime) - Partially done when logging
* Linux support (Maybe it works already, haven't tested yet)
* Record video and audio

## Out-of-scope Features for now / Funcionalidad fuera de planes por el momento

* Soporte para link / transfer (multiplayer)
* Soporte para otros formatos comprimidos (ej: .7z)
* Emulación de periféricos (Camara, impresora, etc)
* Emular juegos beta o prototipos

----

* Support for link / transfer (multiplayer)
* Support for other zip formatos (e.g. .7z)
* Emulation of periferals (Camera, printer, etc)
* Emulation of beta or prototype games

## Contributing / Contribuciones

Este es un proyecto que empecé para aprender así que estoy trabajando solo, pero podés mandarme comentarios a diegoslts@gmail.com si tenés alguna sugerencia.

----

This is a project I started to learn so I'm working alone, but feel free to send comments to diegoslts@gmail.com if you have any suggestion.

## Author / Autor

* **Diego Juodziukynas** - [DiegoSLTS](https://github.com/DiegoSLTS)

Todo esto está basado en información técnica pública que encontré usado Google. La mayoría de los links que usé están acá: https://github.com/gbdev/awesome-gbdev

----

All this is based on public technical information found on Google. Here's almost everything that you'll need: https://github.com/gbdev/awesome-gbdev

## License / Licencia

Este proyecto usa la licencia MIT - ver el archivo [LICENSE.md](LICENSE.md) para mas información

----

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments / Reconocimientos

* Gracias a todas las personas que encontraron y recopilaron toda la información disponible sobre el GameBoy
* Y gracias al equipo que desarrolló BGB por hacer un emulador muy bueno con herramientas de debug muy útiles

----

* Thanks to everyone that compiled and found all the information available about the GameBoy
* Also to the BGB team for making a great GB emulator with really usefull debug tools
