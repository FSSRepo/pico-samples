#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Uso: %s <imagen_entrada> <header_salida> [nombre_var]\n", argv[0]);
        printf("  Ejemplo: %s icono.png icono.h ICONO\n", argv[0]);
        return 1;
    }

    const char* input_path = argv[1];
    const char* output_path = argv[2];
    const char* var_name = (argc >= 4) ? argv[3] : "IMAGE";

    int width, height, channels;
    unsigned char* img = stbi_load(input_path, &width, &height, &channels, 0);
    if (!img) {
        fprintf(stderr, "Error al cargar imagen: %s\n", stbi_failure_reason());
        return 1;
    }

    printf("Imagen cargada: %dx%d, canales=%d\n", width, height, channels);

    FILE* f = fopen(output_path, "w");
    if (!f) {
        fprintf(stderr, "Error al crear archivo de salida\n");
        stbi_image_free(img);
        return 1;
    }

    char guard[64];
    snprintf(guard, sizeof(guard), "%s_H", var_name);

    fprintf(f, "#ifndef %s\n", guard);
    fprintf(f, "#define %s\n\n", guard);
    fprintf(f, "#include <stdint.h>\n\n");
    fprintf(f, "#define %s_WIDTH  %d\n", var_name, width);
    fprintf(f, "#define %s_HEIGHT %d\n\n", var_name, height);
    fprintf(f, "static const uint16_t %s_DATA[] = {\n", var_name);

    for (int y = 0; y < height; y++) {
        fprintf(f, "    ");
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * channels;
            uint8_t r = img[idx + 0];
            uint8_t g = img[idx + 1];
            uint8_t b = img[idx + 2];
            uint8_t a = (channels >= 4) ? img[idx + 3] : 255;

            uint16_t pixel;
            if (a < 128) {
                pixel = 0x0000; // transparente -> negro
            } else {
                pixel = rgb888_to_rgb565(r, g, b);
            }

            fprintf(f, "0x%04X", pixel);
            if (x < width - 1 || y < height - 1) {
                fprintf(f, ", ");
            }
        }
        fprintf(f, "\n");
    }

    fprintf(f, "};\n\n");
    fprintf(f, "#endif // %s\n", guard);
    fclose(f);

    printf("Header generado: %s (%d x %d pixeles RGB565)\n", output_path, width, height);

    stbi_image_free(img);
    return 0;
}
