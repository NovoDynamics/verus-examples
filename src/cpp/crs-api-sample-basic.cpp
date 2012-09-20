#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* include the declaration of API functions and constants */
#include "crs-api.h"

#define MAXPAGES (16)

/* utility functions defined at end of file */
void check_for_error(int errVal);
void usage(int argc, char **argv);

int main(int argc, char **argv)
{
  char path[2048];
  char* install_dir;
  char* input;
  char* basename;

  usage(argc, argv); /* check command line and print usage if needed */

  install_dir = argv[1];
  input = argv[2];
  basename = argv[3];

  /*************************************************************************/
  /* System Initialization                                                 */
  /*************************************************************************/
  check_for_error( crs_properties_initialize(install_dir) );
  check_for_error( crs_system_initialize() );

  /****************************************************************************/
  /* Supply the input parameters to the API,                                  */
  /* execute processing phases and retrieve the output parameters.            */
  /****************************************************************************/

  /* Try to find the basename for the input image (remove suffix) */
  printf("Basename is %s\n",basename);

  /* supply the path of the input image - this is required */
  check_for_error( crs_set_input_path(input) );

  /* get number of pages in this document */
  int pageCount = 0;
  check_for_error( crs_get_input_page_count(&pageCount) );
  printf( "Page Count: %d\n",pageCount);

  /*************************************/
  /* process each page of the document */
  /*************************************/
  for (int pageNum = 0;  pageNum < pageCount;  ++pageNum)
  {
    /*********************************************************/
    /* prepare the processing of document page               */
    /*********************************************************/
    check_for_error( crs_page_initialize() );

    /*********************************************************/
    /* Set the path names of desired output files            */
    /*********************************************************/

    /* supply the path for the cleaned image - this is optional
       The cleaned file is scaled copy of the original image with noise filtered,
       orientation corrected */
    sprintf(path, "%s-page%03d-cln.tif", basename, pageNum);
    check_for_error( crs_set_cleaned_output_path(path) );

    /* supply the path for the recognized text in 8-bit Unicode format -
       this is optional. Note: ASCII is a subset of 8-bit Unicode */
    sprintf(path, "%s-page%03d.utf8", basename, pageNum);
    check_for_error( crs_set_utf8_output_path(path) );

    /* supply the path for the recognized text in 16-bit Unicode format -
       this is optional */
    sprintf(path, "%s-page%03d.utf16", basename, pageNum);
    check_for_error( crs_set_utf16_output_path(path) );

    /*********************************************************/
    /* Recognize, this can be broken up into 4 steps to do   */
    /* less processing if you don't want to do a full OCR    */
    /*********************************************************/
    check_for_error( crs_recognize() );

    /* This returns the language found and being used for aiding in text recognition */
    char language_found[CRS_STRING_SIZE];
    check_for_error( crs_get_language_found(language_found, sizeof(language_found)) );

    /* show the information from phase 3 */
    printf("Language = %s\n", language_found);

    /**********************************************************************/
    /* Write the desired output to files                                  */
    /*                                                                    */
    /* Once crs_write_output() is complete, all the file name parameters  */
    /* are reset and all the intermediate results are cleared.            */
    /*                                                                    */
    /* The program must start again with calls to crs_page_initialize()   */
    /* and crs_phase1_preprocess_and_clean_page()                         */
    /**********************************************************************/
    check_for_error( crs_write_output() );
  }

  /*********************************************************/
  /* System shutdown  - release the allocated resources    */
  /*********************************************************/
  crs_system_shutdown();

  return 0;
}

/********************************************************************************/
/* This is the error handling routine.   It retrieves the text of the error for */
/* the API call and terminates the program.                                     */
/********************************************************************************/
void check_for_error(int errVal)
{
  if (errVal != 0)
  {
    char error_message[CRS_IMAGENAME_SIZE];

    printf("ERROR:\n");
    printf("\tcode: %d\n", errVal);

    /* call the API to retrieve the text of the error.  This call never fails. */
    crs_get_error_text(error_message,            /* buffer to receive the message */
                       sizeof(error_message));   /* size of buffer */
    printf("\ttext: %s\n", error_message);

    crs_system_shutdown();
    exit(1);
  }
}

/********************************************************************************/
/* This function checks that the required input file name is supplied           */
/********************************************************************************/
void usage(int argc, char **argv)
{
  if (argc != 4)
  {
    printf("\nUsage: %s <installation-directory> <input-file-path> <output-base>\n", argv[0]); 
    printf("\nExample: %s C:\\NovoDynamics\\NovoVerus newspaper.tif newspaper-out\n", argv[0]);
    exit(1);
  }
}
