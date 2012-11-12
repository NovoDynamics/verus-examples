using System;
using System.Text;
using Ndi.Verus;

namespace Ndi.VerusApiSample
{
  class VerusApiSampleBasic
  {

    static void process_document(bool do_language_id, String input)
    {
      /* supply the path of the input image - this is required */
      check_for_error( VerusApi.crs_set_input_path(input) );

      /* get number of pages in this document */
      int pageCount = 0;
      check_for_error( VerusApi.crs_get_input_page_count(ref pageCount) );

      /*************************************/
      /* process each page of the document */
      /*************************************/
      for (int pageNum = 0; pageNum < pageCount; ++pageNum)
      {
        Console.Write("File={0} Page={1} ", input, pageNum);
        /*********************************************************/
        /* prepare the processing of document page               */
        /*********************************************************/
        check_for_error( VerusApi.crs_page_initialize() );

        /*********************************************************/
        /* Phase 1. Script ID is done here.                      */
        /* If all you want is the script, you can stop here and  */
        /* continue on to the next page                          */
        /*********************************************************/
        check_for_error( VerusApi.crs_phase1_preprocess_and_clean_page() );
        StringBuilder alphabet_found = new StringBuilder(VerusConstants.CRS_STRING_SIZE);
        check_for_error( VerusApi.crs_get_alphabet_found(alphabet_found, VerusConstants.CRS_STRING_SIZE) );
        Console.Write("Alphabet=" + alphabet_found + " ");

        if(do_language_id)
        {
          check_for_error( VerusApi.crs_phase2_decompose_page() );
          check_for_error( VerusApi.crs_phase3_identify_language() );

          /* retrieve lang id from executing phase 3 */
          /* This returns the language found and being used for aiding in text recognition */
          StringBuilder language_found = new StringBuilder(VerusConstants.CRS_STRING_SIZE);
          check_for_error( VerusApi.crs_get_language_found(language_found, VerusConstants.CRS_STRING_SIZE) );

          /* show the information from phase 3 */
          Console.Write("Language=" + language_found + " ");
        }
        Console.WriteLine();
      }
    }

    [STAThread]
      static void Main(string[] args)
      {
        usage(args); /* check command line and print usage if needed */

        String installation_dir = args[0];
        bool do_language_id = Boolean.Parse(args[1]);

        /*************************************************************************/
        /* System Initialization                                                 */
        /* validates the license file and allocate the needed resources (memory) */
        /*************************************************************************/
        check_for_error( VerusApi.crs_properties_initialize( installation_dir) );
        check_for_error( VerusApi.crs_system_initialize() );

        /* Process each page on command line */
        for(int i = 2; i < args.Length; i++)
        {
          process_document(do_language_id, args[i]);
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
      if (args.Length < 2)
      {
        Console.WriteLine("Usage: ScriptLanguageId.exe <installation-directory> <process-language-id=True|False> <input-file-path> [<input-file-path> ... ] ");
        Console.WriteLine("Example: ScriptLanguageId.exe C:\\NovoDynamics\\NovoVerus True newspaper.tif");
        Environment.Exit(1);
      }
    }
  }
}
