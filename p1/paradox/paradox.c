// Mark Coatsworth
// CS537 Introduction to Operating Systems
// Project 1
// September 16, 2014

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_TRIALS 1000

// Function declarations

int SameBirthdayTrial(int _N);

// Main function
int main(int argc, char **argv)
{
	// Declare program variables
	char *InputFileName = NULL;
	char *OutputFileName = NULL;
	FILE *InputFile;
	FILE *OutputFile;	

	// Determine the filenames of the input file and output file
	int i;
	for(i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-i") == 0)
        {
        	InputFileName = argv[i+1];
        }
        if(strcmp(argv[i], "-o") == 0)
        {
        	OutputFileName = argv[i+1];
        }
    }
    
    // Error checking on input and output filenames
    if(!InputFileName && !OutputFileName)
    {
    	fprintf(stderr, "Error: input and output files not specified.\nUsage: $ ./paradox -i <inputfile> -o <outputfile>\n");
		return 0;
    }
    else if(!InputFileName)
    {
    	fprintf(stderr, "Error: input file not specified.\nUsage: $ ./paradox -i <inputfile> -o <outputfile>\n");
		return 0;
    }
    else if(!OutputFileName)
    {
    	fprintf(stderr, "Error: output file not specified.\nUsage: $ ./paradox -i <inputfile> -o <outputfile>\n");
		return 0;
    }
    
    // Seed the random number generator. Only do this once at the beginning of the program.
	srand(time(NULL));
	
	// Open input file for reading, open output file for writing
	InputFile = fopen(InputFileName, "r");
	OutputFile = fopen(OutputFileName, "w");

	// Read numbers from input file, assuming each number is on a single line
	if (InputFile != NULL)
	{
		int InputValue;
		while(fscanf(InputFile, "%d\n", &InputValue) != EOF)
		{
			if(!(InputValue >= 1 && InputValue <= 365))
			{
				fprintf(stderr, "Error: input values must be integers between 1 and 365\n");
				return 0;
			}
			else
			{
				int NumPositiveTrials = 0;
				float ProbMatchBirthdays;
				
				// Run the trials and determine number of positive trials
				int trial;
				for(trial = 0; trial < NUM_TRIALS; trial ++)
				{
					if(SameBirthdayTrial(InputValue) == 1)
					{
						NumPositiveTrials++;
					}
				}
				
				ProbMatchBirthdays = (float)NumPositiveTrials / (float)NUM_TRIALS;
				
				// Write result to output file
				fprintf(OutputFile, "%.2f\n", ProbMatchBirthdays);
			}
		}
		fclose(InputFile);
	}
	else
	{
		fprintf(stderr, "Error: could not open input file.\n");
		return 0;
	}
	
	// All done!
	return 0;
}

// Determine if two people in a group of size _N have the same birthday, using random birthdays
// Inputs:
//	_N: size of the group of people
// Outputs:
//	0 if no same birthday
//	1 if same birthday

int SameBirthdayTrial(int _N)
{
	int Birthdays[365] = {0};
	int RandomBirthday;
	
	int i;


	
	for(i = 0; i < _N; i ++)
	{
		// Generate a random number (birthday) seeded with the current time
		
		RandomBirthday = rand() % 365;
		
		// Check if this birthday has already occurred. If not, set it in the birthdays array. If it has occurred, return 1.
		if(Birthdays[RandomBirthday] == 0)
		{
			Birthdays[RandomBirthday] = 1;
		}
		else
		{
			return 1;
		}
	} 
	
	// If we got this far without a match, then no birthdays match. Return 0.
	return 0;
}