#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("Usage:\n");
    printf("./main <input file> <output file>\n");
    exit(0);
  }

  char *input_file_name = argv[1];
  char *output_file_name = argv[2];

  FILE *input_file = fopen(input_file_name, "r");
  FILE *output_file = fopen(output_file_name, "w");

  if (input_file == NULL)
  {
    printf("Error: unable to open input file %s\n", input_file_name);
    exit(0);
  }

  if (output_file == NULL)
  {
    printf("Error: unable to open output file %s\n", output_file_name);
    fclose(input_file);
    exit(0);
  }

  int image_column, image_row, kernel_column, kernel_row;
  fscanf(input_file, "%d %d", &image_column, &image_row);

  int *image = (int *)malloc(image_row * image_column * sizeof(int));
  for (int i = 0; i < image_row * image_column; i++)
  {
    fscanf(input_file, "%d", &image[i]);
  }

  fscanf(input_file, "%d %d", &kernel_column, &kernel_row);

  int *kernel = (int *)malloc(kernel_row * kernel_column * sizeof(int));
  for (int i = 0; i < kernel_row * kernel_column; i++)
  {
    fscanf(input_file, "%d", &kernel[i]);
  }

  int result_column = image_column - kernel_column + 1;
  int result_row = image_row - kernel_row + 1;

  int i = 0;
  while (i < result_row)
  {
    int j = 0;
    while (j < result_column)
    {
      int sum = 0;
      int ki = 0;
      while (ki < kernel_row)
      {
        int kj = 0;
        while (kj < kernel_column)
        {
          sum += image[(i + ki) * image_column + (j + kj)] * kernel[ki * kernel_column + kj];
          kj++;
        }
        ki++;
      }
      fprintf(output_file, "%d ", sum);
      j++;
    }
    fprintf(output_file, "\n");
    i++;
  }

  free(image);
  free(kernel);

  // int image_column, image_row, kernel_column, kernel_row;
  // fscanf(input_file, "%d %d", &image_column, &image_row);

  // int *image = (int *)malloc(image_row * image_column * sizeof(int));
  // int flag = 0;
  // while (flag < image_row * image_column)
  // {
  //   fscanf(input_file, "%d", &image[flag]);
  //   flag++;
  // }

  // fscanf(input_file, "%d %d", &kernel_column, &kernel_row);

  // int *kernel = (int *)malloc(kernel_row * kernel_column * sizeof(int));
  // flag = 0;
  // while (flag < kernel_row * kernel_column)
  // {
  //   fscanf(input_file, "%d", &kernel[flag]);
  //   flag++;
  // }

  // int pad_size = (kernel_row - 1) / 2;
  // int padded_column = image_column + 2 * pad_size;
  // int padded_row = image_row + 2 * pad_size;

  // int *padded_image = (int *)calloc(padded_row * padded_column, sizeof(int));
  // for (int i = 0; i < image_row; i++)
  // {
  //   for (int j = 0; j < image_column; j++)
  //   {
  //     padded_image[(i + pad_size) * padded_column + (j + pad_size)] = image[i * image_column + j];
  //   }
  // }

  // int i = 0;
  // while (i < image_row)
  // {
  //   int j = 0;
  //   while (j < image_column)
  //   {
  //     int sum = 0;
  //     int ki = 0;
  //     while (ki < kernel_row)
  //     {
  //       int kj = 0;
  //       while (kj < kernel_column)
  //       {
  //         sum += padded_image[(i + ki) * padded_column + (j + kj)] * kernel[ki * kernel_column + kj];
  //         kj++;
  //       }
  //       ki++;
  //     }
  //     fprintf(output_file, "%d ", sum);
  //     j++;
  //   }
  //   fprintf(output_file, "\n");
  //   i++;
  // }

  // free(image);
  // free(kernel);
  // free(padded_image);

  /* YOUR CODE HERE */

  fclose(input_file);
  fclose(output_file);

  return 0;
}
