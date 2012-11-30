#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "crs-api.h"

using std::string;
using std::cout;
using std::endl;
using std::stringstream;

/********************************************************************************/
/* This is the error handling routine.   It retrieves the text of the error for */
/* the API call and terminates the program.                                     */
/********************************************************************************/
void check_for_error(int errVal)
{
  if (errVal != 0)
  {
    char *error_message = new char[CRS_STRING_SIZE];

    cout << "ERROR:" << endl;
    cout << "\tcode: " << errVal << endl;

    /* call the API to retrieve the text of the error.  This call never fails. */
    crs_get_error_text(error_message,            /* buffer to receive the message */
                    CRS_STRING_SIZE);            /* size of buffer */
    cout << "\ttext: " << error_message << endl;
    delete []error_message;

    crs_system_shutdown();
    exit(1);
  }
}

/********************************************************************************/
/* This function checks that the required input file name is supplied           */
/********************************************************************************/
void usage(int argc)
{
  if (argc < 2)
  {
    cout << "Usage: ScriptLanguageId.exe <installation-directory> <process-language-id=1|0> <input-file-path> [<input-file-path> ... ] " << endl;
    cout << "Example: ScriptLanguageId.exe C:\\NovoDynamics\\NovoVerus True newspaper.tif" << endl;
    exit(1);
  }
}

void process_document(int index, bool do_language_id, const string& input)
{
  /* supply the path of the input image - this is required */
  check_for_error( crs_set_input_path(input.c_str()) );

  /* get number of pages in this document */
  int pageCount = 0;
  check_for_error( crs_get_input_page_count(&pageCount) );

  /*************************************/
  /* process each page of the document */
  /*************************************/
  for (int pageNum = 0; pageNum < pageCount; ++pageNum)
  {
    cout << "File=" << input << " Page=" << pageNum << " ";
    /*********************************************************/
    /* prepare the processing of document page               */
    /*********************************************************/
    check_for_error( crs_page_initialize() );


    //CRS_DISABLE/CRS_ENABLE
    check_for_error( crs_set_page_orientation_correction_mode(CRS_DISABLE));
    check_for_error( crs_set_page_cropping_mode(CRS_DISABLE));
    check_for_error( crs_set_page_deskewing_correction_mode(CRS_DISABLE));
    check_for_error( crs_set_extendedarabic_handwriting_processing_mode(CRS_ENABLE));

    check_for_error( crs_set_noise_filter(0)); //noise filter = 1-7, 0 == disable

    /*********************************************************/
    /* Phase 1. Script ID is done here.                      */
    /* If all you want is the script, you can stop here and  */
    /* continue on to the next page                          */
    /*********************************************************/
    check_for_error( crs_phase1_preprocess_and_clean_page() );
    char* alphabet_found = new char[CRS_STRING_SIZE];
    check_for_error( crs_get_alphabet_found(alphabet_found, CRS_STRING_SIZE) );
    cout << "Alphabet=" << alphabet_found << " ";
    delete []alphabet_found;

    if(do_language_id)
    {
      check_for_error( crs_phase2_decompose_page() );
      check_for_error( crs_phase3_identify_language() );

      /* retrieve lang id from executing phase 3 */
      /* This returns the language found and being used for aiding in text recognition */
      char* language_found = new char[CRS_STRING_SIZE];
      memset(language_found, 0, sizeof(language_found));
      check_for_error( crs_get_language_found(language_found, CRS_STRING_SIZE) );

      /* show the information from phase 3 */
      cout << "Language=" << language_found << " ";
      delete []language_found;
    }
    cout << endl;

    stringstream ss;
    ss << "cleanedimg-" << index << "-" << pageNum << ".tif";
    string clean_path = ss.str();
    cout << "CleanedImage=" << clean_path << endl;
    check_for_error( crs_set_cleaned_output_path(clean_path.c_str()) );
    check_for_error( crs_write_output() );
  }
}

int main(int argc, char **argv)
{
  usage(argc); /* check command line and print usage if needed */

  string installation_dir(argv[1]);
  bool do_language_id = bool(atoi(argv[2]));

  /*************************************************************************/
  /* System Initialization                                                 */
  /* validates the license file and allocate the needed resources (memory) */
  /*************************************************************************/
  check_for_error( crs_properties_initialize( installation_dir.c_str() ) );
  check_for_error( crs_system_initialize() );

  /* Process each page on command line */
  for(int i = 3; i < argc; i++)
  {
    process_document(i-3, do_language_id, argv[i]);
  }

  /*********************************************************/
  /* System shutdown  - release the allocated resources    */
  /*********************************************************/
  crs_system_shutdown();

  return 0;
}

