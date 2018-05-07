
# JpegDecoder

## Environment
- Machine: MacBook Air (13-inch, Early 2015)
- OS: macOS Sierra 10.12.3

## Compile
    $ make
## Run
    $ ./main <filePath1> [... <filePathN>]

## File Structure

### Main Programs
- **main.c**
- **JpegToBmp.c**
    - jpegToBmp();
        - **JpegDecoder.c**
            - freadJpeg();
                - **JpegHeader.c**
                    - fparseJpegHeader();
                - **JpegBody.c**
                    - fdecodeJpegBody();
                        - **JpegHuffman.c**
                            - fdecodeHuffman();
        - MCUListToRGBMatrix();
        - **BmpWriter.c**
            - fwriteBmp();
                - fwriteBmpFileHeader();
                - fwriteBmpInfoHeader();
                - fwriteBmpBody();
            
### Main Objects
- **JpegMCU.c**
- **RGBMatrix.c**

### Other Tools
- **HuffmanTree.c**
- **FastIDCT.c**
- **BitOperationTools.c**
- **ErrorHandler.c**

### System Libraries
- **<stdio.h>**
- **<stdlib.h>**
- **<string.h>**
- **<math.h>**
- **<assert.h>**