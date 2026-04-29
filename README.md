# NI-GPU

Semestrální práce z předmětu NI-GPU zabývající se paralelizací Cannyho hranového detektoru.

## Obsah

- cuda - složka s paralelním řešením
- images - složka se vstupními obrázky
- output - složka s výpisy naměřených běhů
- sequential - složka se sekvenčním řešením
- report.pdf

## Kompilace

Řešení mají ve svých složkách vlastní Makefile, takže stačí jít do dané složky a použít příkaz make. Po kompilaci vzniknou následující programy:

- canny_sequential
- canny_cuda

## Spuštění

Sekvenční:

    ./canny_sequential <image_path> <sigma> <lower_threshold> <upper_threshold>

Paralelní:

    ./canny_cuda <image_path> <sigma> <lower_threshold> <upper_threshold> <2D_block_width> <2D_block_height> <1D_block_size>

- **image_path** - cesta k obrázku
- **sigma** - parametr sigma pro Gaussovské rozostření
- **lower_threshold** a **upper_threshold** - spodní a horní práh pro dvojité prahování (procentuální hodnota od 0.0 do 1.0)
- **2D_block_width** a **2D_block_height** - rozměry dvourozměrných bloků vláken
- **1D_block_size** - velikost jednorozměrných bloků vláken
