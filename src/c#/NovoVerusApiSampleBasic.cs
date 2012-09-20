using System;
using System.Text;
using Ndi.Verus;

namespace Ndi.VerusApiSample
{
  class VerusApiSampleBasic
  {
      [STAThread]
      static void Main(string[] args)
      {
        usage(args); /* check command line and print usage if needed */

        String installation_dir = args[0];
        String input = args[1];
        String basename = args[2];

        /*************************************************************************/
        /* System Initialization                                                 */
        /* validates the license file and allocate the needed resources (memory) */
        /*************************************************************************/
        check_for_error( VerusApi.crs_properties_initialize( installation_dir) );
        check_for_error( VerusApi.crs_system_initialize() );

        /****************************************************************************/
        /* Supply the input parameters to the API,                                  */
        /* execute processing phases and retrieve the output parameters.            */
        /****************************************************************************/

        /* supply the path of the input image - this is required */
        check_for_error( VerusApi.crs_set_input_path(input) );

        /* get number of pages in this document */
        int pageCount = 0;
        check_for_error( VerusApi.crs_get_input_page_count(ref pageCount) );
        Console.WriteLine("Page Count: " + pageCount);

        /*************************************/
        /* process each page of the document */
        /*************************************/
        for (int pageNum = 0; pageNum < pageCount; ++pageNum)
        {
          /*********************************************************/
          /* prepare the processing of document page               */
          /*********************************************************/
          check_for_error( VerusApi.crs_page_initialize() );

          /*********************************************************/
          /* Set the path names of desired output files            */
          /*********************************************************/

          /* supply the path for the cleaned image - this is optional
             The cleaned file is scaled copy of the original image with noise filtered,
             orientation corrected */
          String cleanpath = basename + "-" + "page" + pageNum + "-clean.tif";
          check_for_error( VerusApi.crs_set_cleaned_output_path(cleanpath) );

          /* supply the path for the recognized text in 8-bit Unicode format -
             this is optional. Note: ASCII is a subset of 8-bit Unicode */
          String utf8path = basename + "-" + "page" + pageNum + ".utf8";
          check_for_error( VerusApi.crs_set_utf8_output_path(utf8path) );

          /* supply the path for the recognized text in 16-bit Unicode format -
             this is optional */
          String utf16path = basename + "-" + "page" + pageNum + ".utf16";
          check_for_error( VerusApi.crs_set_utf16_output_path(utf16path) );

          /****************************************************************************/
          /* Supply the input parameters to the API,                                  */
          /* execute processing phases and retrieve the output parameters.            */
          /****************************************************************************/

          /*********************************************************/
          /* Recognize, this can be broken up into 4 steps to do   */
          /* less processing if you don't want to do a full OCR    */
          /*********************************************************/
          check_for_error( VerusApi.crs_recognize() );

          /* retrieve information from executing phase 3 */
          StringBuilder language_found = new StringBuilder(VerusConstants.CRS_STRING_SIZE);

          /* This returns the language found and being used for aiding in text recognition */
          check_for_error( VerusApi.crs_get_language_found(language_found, VerusConstants.CRS_STRING_SIZE) );

          /* show the information from phase 3 */
          Console.WriteLine("Language = " + language_found);

          /**********************************************************************/
          /* Write the desired output to files                                  */
          /*                                                                    */
          /* Once crs_write_output() is complete, all the file name parameters  */
          /* are reset and all the intermediate results are cleared.            */
          /*                                                                    */
          /* The program must start again with calls to crs_page_initialize()   */
          /* and crs_phase1_preprocess_and_clean_page()                         */
          /**********************************************************************/
          check_for_error( VerusApi.crs_write_output() );
        }

        /*********************************************************/
        /* System shutdown  - release the allocated resources    */
        /*********************************************************/
        VerusApi.crs_system_shutdown();
      }

      /********************************************************************************/
      /* This is the error handling routine.   It retrieves the text of the error for */
      /* the API call and terminates the program.                                     */
      /********************************************************************************/
      static void check_for_error(int errVal)
      {
        if (errVal != 0)
        {
          StringBuilder error_message = new StringBuilder(VerusConstants.CRS_STRING_SIZE);

          Console.WriteLine("ERROR:");
          Console.WriteLine("\tcode: " + errVal);

          /* call the API to retrieve the text of the error.  This call never fails. */
          VerusApi.crs_get_error_text(error_message,            /* buffer to receive the message */
                                      VerusConstants.CRS_STRING_SIZE);   /* size of buffer */
          Console.WriteLine("\ttext: " + error_message);

          VerusApi.crs_system_shutdown();
          Environment.Exit(1);
        }
      }

      /********************************************************************************/
      /* This function checks that the required input file name is supplied           */
      /********************************************************************************/
      static void usage(string[] args)
      {
        if (args.Length != 3)
        {
          Console.WriteLine("Usage: NovoVerusExample.exe <installation-directory> <input-file-path> <output-base>");
          Console.WriteLine("Example: NovoVerusExample.exe C:\\NovoDynamics\\NovoVerus newspaper.tif newspaper-out");
          Environment.Exit(1);
        }
      }
  }
}
