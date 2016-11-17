#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <time.h>

using namespace std;

//3 things that must be manually changed for every new instance
const int TOTALTYPEOFSHIFTS = 2;
const int TOTALNUMBEROFNURSES = 14;
const int TOTALNUMBEROFDAYS = 14;

const int SELFCONSTRAINTMINWORKINGDAYSAWEEK = 2;

const int ALLPOSSIBLECOMBINATIONS = 128; // for 2 shifts 2 power 7 = 128

const int MAXWEEKLYSCPENALTYCOST = 700;
const int MINWEEKLYSCPENALTYCOST = 0;
const int MAXFINALPENALTYCOST = 1000;

const int INDIVIDUALNURSEPATTERNPENALTYCOSTRANGE = 0; //range of penalty costs increase available from zero cost

const int MAXNUMBEROFWEEKLYPATTERNSGENERATED = 10000;

const int WEEKNUMBEROFDAYS = 7;

string shiftID[TOTALTYPEOFSHIFTS];
int shiftMinutesLengthInfo[TOTALTYPEOFSHIFTS] = { 0 };

//this is the original string info read from the conditions file
string nextShiftNotAllowedInfo[TOTALTYPEOFSHIFTS][TOTALTYPEOFSHIFTS]; 
//string info converted to int info 
//TOTALTYPEOFSHIFTS + 1 index is used to store array length of this shift
int nextShiftNotAllowedIntegerFormArray[TOTALTYPEOFSHIFTS][TOTALTYPEOFSHIFTS + 1] = { 0 };

int maxConsecShift[TOTALNUMBEROFNURSES] = { 0 };
int minConsecShift[TOTALNUMBEROFNURSES] = { 0 };

int minConsecDaysOff[TOTALNUMBEROFNURSES] = { 0 };

int maxWeekends[TOTALNUMBEROFNURSES] = { 0 };

string nurseID[TOTALNUMBEROFNURSES];
int maxShiftsPerType[TOTALNUMBEROFNURSES][TOTALTYPEOFSHIFTS] = { 0 };

int maxMinutesPerNurse[TOTALNUMBEROFNURSES] = { 0 };
int minMinutesPerNurse[TOTALNUMBEROFNURSES] = { 0 };

int shiftHCDaysOff[TOTALNUMBEROFNURSES][TOTALNUMBEROFDAYS] = { 0 }; // -1 if no days off

//index 0 is shift cover amount
//index 1 is shift penalty weight for under supply
//index 2 is shift penalty weight for over supply
int shiftCoverAmount[TOTALTYPEOFSHIFTS][3][TOTALNUMBEROFDAYS] = { 0 };

int NumberOfShiftsInADay[2][WEEKNUMBEROFDAYS] = { 0 }; //index 0 is for total number of shifts, index 1 is for comparison between section cover request


int NurseAcceptedCoverCombinations[TOTALNUMBEROFNURSES][ALLPOSSIBLECOMBINATIONS] = { 0 }; //contains the index of accepted combinations in FinalCombinationsArray

int FinalCombinationsArray[ALLPOSSIBLECOMBINATIONS][WEEKNUMBEROFDAYS] = { 0 }; // all possible combinations
int FinalCombinationsArrayReverse[ALLPOSSIBLECOMBINATIONS][WEEKNUMBEROFDAYS] = { 0 };

//this is priority on days to work
//numbers in array are weights in day order (higher weight, higher priority)
int ShiftOnRequest[TOTALTYPEOFSHIFTS][TOTALNUMBEROFNURSES][TOTALNUMBEROFDAYS] = { 0 };

//this is priority on rest days
//numbers in array are weights in day order (higher weight, higher priority)
int ShiftOffRequest[TOTALTYPEOFSHIFTS][TOTALNUMBEROFNURSES][TOTALNUMBEROFDAYS] = { 0 };

////////////////new code

//WEEKNUMBEROFDAYS starts from index 0, last extra index is used for penalty cost value
//ALLPOSSIBLECOMBINATIONS + 1 stores the actual number of occupied array indexes
int nurseHCPatterns[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES][TOTALTYPEOFSHIFTS][ALLPOSSIBLECOMBINATIONS + 1][WEEKNUMBEROFDAYS + 1] = { 0 };

//TOTALNUMBEROFNURSES + 1, extra array slot is used to store pattern penalty costs at (index 0 of 3rd dimension)
int allCostsFullWeekFinalPatternsArray[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][MAXNUMBEROFWEEKLYPATTERNSGENERATED][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };
int allCostsArrayCounter = 0;

int currentNursePatternGenerationIndex[TOTALNUMBEROFNURSES] = { 0 };

//TOTALNUMBEROFNURSES + 1, last index is used to store shift amount penalty value
//WEEKNUMBEROFDAYS + 1, last index is used to store SECTION_DAY_ON and OFF penalty value
int currentNursePatternGenerationTempArray[TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 }; 

int allAcceptedFinalScheduleArray[MAXNUMBEROFWEEKLYPATTERNSGENERATED][TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };
int allAcceptedFinalScheduleArrayCounter = 0;

int tempScheduleOutput[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };

int allWeeksAcceptedPatternIndex[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES][MAXNUMBEROFWEEKLYPATTERNSGENERATED + 1] = { 0 };

//new items
int hillClimbingPreliminarySchedule[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };
int greedySearchPreliminarySchedule[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };

int tempMergeShiftSchedule[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][TOTALTYPEOFSHIFTS][WEEKNUMBEROFDAYS + 1] = { 0 };

int finalScheduleOutput[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][TOTALTYPEOFSHIFTS][WEEKNUMBEROFDAYS + 1] = { 0 };
int finalSchedule[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };

//variable -1 means free day, variable -2 means occupied day, positive variable means unavailable day of shift (variable) due to nextShiftUnavailable 
int availableDaySlots[WEEKNUMBEROFDAYS] = { 0 };

int listOfAcceptedPatternsAllWeeks[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES][TOTALTYPEOFSHIFTS][ALLPOSSIBLECOMBINATIONS + 1] = { 0 };
int currentPatternSelectionAllWeeks[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES][TOTALTYPEOFSHIFTS] = { 0 };

int allNurseShiftRankingAndPenaltyCost[TOTALNUMBEROFNURSES][TOTALTYPEOFSHIFTS][2] = { 0 };
int testedNurseFirstShiftScheduledSelectedPattern[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES][ALLPOSSIBLECOMBINATIONS + 1] = { 0 };

ifstream patternDataFileStream;
ofstream weeklyDataFileStream;
fstream finalScheduleAnswerStream;
ifstream scheduleConditionsStream;

int currentScheduleGenerationIndex[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS] = { 0 };

bool isNumeric(const std::string& input) {
	return std::all_of(input.begin(), input.end(), ::isdigit);
}

void ReadInputParametersFile()
{
	char tempChar;
	string tempLine = "";
	string tempWord = "";
	int tempInt = 0;

	int shiftTypeArrayIndex = 0;
	int shiftTypeArrayHorizontalIndex = 0;
	int nextShiftNotAllowedCounter = 0;

	int sectionStaffArrayIndex = 0;
	int sectionStaffArrayHorizontalIndex = 0;

	//starts from -1, resets to -1
	int maxShiftsPerTypeIndex = -1;
	//neutral val, -1
	int sectionType = -1;

	int sectionDaysOffNurseIndex = 0;
	int sectionDaysOffHorizontalIndex = 0;

	//starts from -1, resets to -1
	int shiftOnRequestNurseNum = -1;
	int shiftOnRequestShiftNum = -1;
	int shiftOnRequestTargetDay = -1;
	//starts from 0
	int shiftOnRequestHorizontalIndex = 0;

	//starts from -1, resets to -1
	int shiftOffRequestNurseNum = -1;
	int shiftOffRequestShiftNum = -1;
	int shiftOffRequestTargetDay = -1;
	//starts from 0
	int shiftOffRequestHorizontalIndex = 0;

	//starts from -1, resets to -1
	int shiftCoverDayNum = -1;
	int shiftCoverShiftNum = -1;
	//starts from 0
	int shiftCoverRequestHorizontalIndex = 0;

	if (scheduleConditionsStream.is_open())
	{
		while (getline(scheduleConditionsStream, tempLine))
		{
			for (int i = 0; i < tempLine.size(); i++)
			{
				tempChar = tempLine[i];

				if ((tempChar != ' ') && (tempChar != ',') && (tempChar != '|') && (tempChar != '='))
				{
					tempWord += tempChar;

					//if this is last character of line
					if (i >= tempLine.size() - 1)
					{
						//is a number
						if (isNumeric(tempWord) == true)
						{
							int tempInt = stoi(tempWord);

							switch (sectionType)
							{
							case -1://info can't be used to change const int
								break;
							case 0:
								break;
							case 1:
								if (sectionStaffArrayHorizontalIndex == 7)
								{
									int tempInt = stoi(tempWord);

									maxWeekends[sectionStaffArrayIndex] = tempInt;
								}
								else
								{
									printf("sectionStaffArrayHorizontalIndex Error 1\n");
								}
								sectionStaffArrayIndex++;
								break;
							case 2:
								shiftHCDaysOff[sectionDaysOffNurseIndex][sectionDaysOffHorizontalIndex] = tempInt;
								shiftHCDaysOff[sectionDaysOffNurseIndex][TOTALNUMBEROFDAYS - 1] = sectionDaysOffHorizontalIndex + 1;
								sectionDaysOffNurseIndex++;
								sectionDaysOffHorizontalIndex = 0;
								break;
							case 3:
								ShiftOnRequest[shiftOnRequestShiftNum][shiftOnRequestNurseNum][shiftOnRequestTargetDay] = tempInt;
								shiftOnRequestNurseNum = -1;
								shiftOnRequestShiftNum = -1;
								shiftOnRequestTargetDay = -1;
								shiftOnRequestHorizontalIndex = 0;
								break;
							case 4:
								ShiftOffRequest[shiftOffRequestShiftNum][shiftOffRequestNurseNum][shiftOffRequestTargetDay] = tempInt;
								shiftOffRequestNurseNum = -1;
								shiftOffRequestShiftNum = -1;
								shiftOffRequestTargetDay = -1;
								shiftOffRequestHorizontalIndex = 0;
								break;
							case 5:
								shiftCoverAmount[shiftCoverShiftNum][2][shiftCoverDayNum] = tempInt;
								shiftCoverDayNum = -1;
								shiftCoverShiftNum = -1;
								shiftCoverRequestHorizontalIndex = 0;
								break;								
							default:
								printf("section type error\n");
								break;
							}
						}
						else //is not a number,
						{
							if (sectionType == 0 && shiftTypeArrayHorizontalIndex >= 2)
							{
								if (shiftTypeArrayHorizontalIndex >= 2)
								{
									nextShiftNotAllowedInfo[shiftTypeArrayIndex][nextShiftNotAllowedCounter] = tempWord;
									nextShiftNotAllowedCounter = 0;
								}
								else
								{
									printf("shiftTypeArray Error 0\n");
								}

								shiftTypeArrayIndex++;
								shiftTypeArrayHorizontalIndex = 0;
								tempWord = "";
							}


							if (tempWord == "SECTION_HORIZON")
							{
								sectionType = -1;
							}
							else if (tempWord == "SECTION_SHIFTS")
							{
								sectionType = 0;
							}
							else if (tempWord == "SECTION_STAFF")
							{
								sectionType = 1;
							}
							else if (tempWord == "SECTION_DAYS_OFF")
							{
								sectionType = 2;
							}
							else if (tempWord == "SECTION_SHIFT_ON_REQUESTS")
							{
								sectionType = 3;
							}
							else if (tempWord == "SECTION_SHIFT_OFF_REQUESTS")
							{
								sectionType = 4;
							}
							else if (tempWord == "SECTION_COVER")
							{
								sectionType = 5;
							}
						}

						//reset tempWord
						tempWord = "";
					}
				}
				else//word is being processed
				{
					//this is a comment
					if (tempWord == "#")
					{
						break;
					}

					switch (sectionType)
					{
					case -1://info can't be used to change const int
						break;
					case 0:
						//is not a number
						if (isNumeric(tempWord) == false)
						{
							if (shiftTypeArrayHorizontalIndex == 0)
							{
								shiftID[shiftTypeArrayIndex] = tempWord;
							}
							else if (shiftTypeArrayHorizontalIndex >= 2)
							{
								nextShiftNotAllowedInfo[shiftTypeArrayIndex][nextShiftNotAllowedCounter] = tempWord;
								nextShiftNotAllowedCounter++;
							}
							else
							{
								printf("shiftTypeArray Error\n");
							}
						}
						else
						{
							int tempInt = stoi(tempWord);

							if (shiftTypeArrayHorizontalIndex == 1)
							{
								shiftMinutesLengthInfo[shiftTypeArrayIndex] = tempInt;
							}
							else
							{
								printf("shiftTypeArray Error 2\n");
							}

							//if this is last character of line
							if (i >= tempLine.size() - 1)
							{
								shiftTypeArrayIndex++;
							}
						}

						shiftTypeArrayHorizontalIndex++;
						tempWord = "";
						break;
					case 1:
						if (sectionStaffArrayHorizontalIndex == 0)
						{
							//check during runtime, not sure if it works
							nurseID[sectionStaffArrayIndex] = tempWord;

							sectionStaffArrayHorizontalIndex++;
						}
						else if (sectionStaffArrayHorizontalIndex == 1)
						{
							//is not a number
							if (isNumeric(tempWord) == false)
							{
								maxShiftsPerTypeIndex++;
							}
							else
							{
								int tempInt = stoi(tempWord);

								maxShiftsPerType[sectionStaffArrayIndex][maxShiftsPerTypeIndex] = tempInt;

								if (maxShiftsPerTypeIndex >= TOTALTYPEOFSHIFTS - 1)
								{
									sectionStaffArrayHorizontalIndex++;
								}
							}
						}
						else if (sectionStaffArrayHorizontalIndex == 2)
						{
							int tempInt = stoi(tempWord);

							maxMinutesPerNurse[sectionStaffArrayIndex] = tempInt;
							sectionStaffArrayHorizontalIndex++;
						}
						else if (sectionStaffArrayHorizontalIndex == 3)
						{
							int tempInt = stoi(tempWord);

							minMinutesPerNurse[sectionStaffArrayIndex] = tempInt;
							sectionStaffArrayHorizontalIndex++;
						}
						else if (sectionStaffArrayHorizontalIndex == 4)
						{
							int tempInt = stoi(tempWord);

							maxConsecShift[sectionStaffArrayIndex] = tempInt;
							sectionStaffArrayHorizontalIndex++;
						}
						else if (sectionStaffArrayHorizontalIndex == 5)
						{
							int tempInt = stoi(tempWord);

							minConsecShift[sectionStaffArrayIndex] = tempInt;
							sectionStaffArrayHorizontalIndex++;
						}
						else if (sectionStaffArrayHorizontalIndex == 6)
						{
							int tempInt = stoi(tempWord);

							minConsecDaysOff[sectionStaffArrayIndex] = tempInt;
							sectionStaffArrayHorizontalIndex++;
						}
						else
						{
							printf("sectionStaffArrayHorizontalIndex Error 2\n");
						}
						//reset tempWord
						tempWord = "";
						break;
					case 2:
						//is not a number
						if (isNumeric(tempWord) == false)
						{
							//do nothing
						}
						else
						{
							int tempInt = stoi(tempWord);

							shiftHCDaysOff[sectionDaysOffNurseIndex][sectionDaysOffHorizontalIndex] = tempInt;

							sectionDaysOffHorizontalIndex++;
						}
						tempWord = "";
						break;
					case 3:
						//is not a number
						if (isNumeric(tempWord) == false)
						{
							if (shiftOnRequestHorizontalIndex == 0)
							{
								for (int j = 0; j < TOTALNUMBEROFNURSES; j++)
								{
									if (tempWord == nurseID[j])
									{
										shiftOnRequestNurseNum = j;
										break;
									}
								}

								if (shiftOnRequestNurseNum < 0)
								{
									printf("nurse num comparison error\n");
								}
							}
							else if (shiftOnRequestHorizontalIndex == 2)
							{
								for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
								{
									if (tempWord == shiftID[j])
									{
										shiftOnRequestShiftNum = j;
										break;
									}
								}

								if (shiftOnRequestShiftNum < 0)
								{
									printf("shift num comparison error\n");
								}
							}
							else
							{
								printf("shiftonrequest horizontal index error\n");
							}
						}
						else
						{
							int tempInt = stoi(tempWord);

							shiftOnRequestTargetDay = tempInt;
						}

						shiftOnRequestHorizontalIndex++;
						tempWord = "";
						break;
					case 4:
						//is not a number
						if (isNumeric(tempWord) == false)
						{
							if (shiftOffRequestHorizontalIndex == 0)
							{
								for (int j = 0; j < TOTALNUMBEROFNURSES; j++)
								{
									if (tempWord == nurseID[j])
									{
										shiftOffRequestNurseNum = j;
										break;
									}
								}

								if (shiftOffRequestNurseNum < 0)
								{
									printf("nurse num shift off comparison error\n");
								}
							}
							else if (shiftOffRequestHorizontalIndex == 2)
							{
								for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
								{
									if (tempWord == shiftID[j])
									{
										shiftOffRequestShiftNum = j;
										break;
									}
								}

								if (shiftOffRequestShiftNum < 0)
								{
									printf("shift num shift off comparison error\n");
								}
							}
							else
							{
								printf("shiftoffrequest horizontal index error\n");
							}
						}
						else
						{
							int tempInt = stoi(tempWord);

							shiftOffRequestTargetDay = tempInt;
						}

						shiftOffRequestHorizontalIndex++;
						tempWord = "";
						break;
					case 5:
						//is not a number
						if (isNumeric(tempWord) == false)
						{
							for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
							{
								if (tempWord == shiftID[j])
								{
									shiftCoverShiftNum = j;
									break;
								}
							}

							if (shiftCoverShiftNum < 0)
							{
								printf("shift cover shift num comparison error\n");
							}
						}
						else
						{
							int tempInt = stoi(tempWord);

							if (shiftCoverRequestHorizontalIndex == 0)
							{
								shiftCoverDayNum = tempInt;
							}
							else if (shiftCoverRequestHorizontalIndex == 2 || shiftCoverRequestHorizontalIndex == 3)
							{
								shiftCoverAmount[shiftCoverShiftNum][shiftCoverRequestHorizontalIndex - 2][shiftCoverDayNum] = tempInt;
							}
							else
							{
								printf("shift cover horizontal index error\n");
							}
						}

						shiftCoverRequestHorizontalIndex++;
						tempWord = "";
						break;
					default:
						printf("section type error version 2\n");
						break;
					}
				}
			}

			//end of line
			switch (sectionType)
			{
			case -1://info can't be used to change const int
				break;
			case 0:
				shiftTypeArrayHorizontalIndex = 0;
				nextShiftNotAllowedCounter = 0;
				break;
			case 1:
				maxShiftsPerTypeIndex = -1;
				sectionStaffArrayHorizontalIndex = 0;
				break;
			case 2:
				sectionDaysOffHorizontalIndex = 0;
				break;
			default:
				break;
			}

			//reset tempWord
			tempWord = "";
		}
	}
	else
	{
		printf("failed to open parameters file\n");
	}

}

//fileStreamNum 1 = patternDataFileStream
void ReadFile(int fileStreamNum)
{
	char tempChar;
	string tempLine;
	string tempWord;
	int tempInt;

	bool isStringInfoLine = false;
	bool isThisNurseInfoComplete = false;
	bool isNumberCompletelyPrinted = false;

	int currentWeekNum = 0;
	int currentNurseNum = 0;
	int currentIntNum = 0;
	int numberOfPatterns = 0;

	int nurseHCPatternsCounter = 0;

	//starts at -1, resets to -1
	int nurseHCPatternsShiftCounter = -1;

	if (fileStreamNum == 0)
	{
		ReadInputParametersFile();
	}
	else if (fileStreamNum == 1)
	{
		if (patternDataFileStream.is_open())
		{
			while (getline(patternDataFileStream, tempLine))
			{
				for (int i = 0; i < tempLine.size(); i++)
				{
					tempChar = tempLine[i];

					if (tempChar != ' ')
					{
						tempWord += tempChar;

						//if this is last character of line
						if (i >= tempLine.size() - 1)
						{
							//is a number
							if (isNumeric(tempWord) == true)
							{
								if (isStringInfoLine == true)
								{
									int tempInt = stoi(tempWord);

									if (isThisNurseInfoComplete == false)
									{
										if (currentIntNum == 0)
										{
											currentWeekNum = tempInt;
										}
										else if (currentIntNum == 1)
										{
											currentNurseNum = tempInt;
										}
										else
										{
											printf("some int reading error\n");
										}
									}
									else //isThisNurseInfoComplete == true
									{
										if (currentIntNum == 0)
										{
										
										}
										else
										{
											printf("some int reading error TYPE 2\n");
										}

									}

									currentIntNum++;
								}
								else//isStringInfoLine is false
								{
									int tempInt = stoi(tempWord);

									nurseHCPatterns[currentWeekNum][currentNurseNum][nurseHCPatternsShiftCounter][nurseHCPatternsCounter][currentIntNum] = tempInt;

									currentIntNum++;
								}

								//reset tempWord
								tempWord = "";
							}
							else //is not a number, is --------------------------- or ++++++++++++++++++++++++++++==
							{
								if (nurseHCPatternsShiftCounter >= 0)
								{
									nurseHCPatterns[currentWeekNum][currentNurseNum][nurseHCPatternsShiftCounter][ALLPOSSIBLECOMBINATIONS][0] = nurseHCPatternsCounter;
									nurseHCPatternsCounter = 0;
								}

								nurseHCPatternsShiftCounter++;
								//reset tempWord
								tempWord = "";
							}
						}
					}
					else
					{
						//is not a number
						if (isNumeric(tempWord) == false)
						{
							if (tempWord == "Week")
							{
								//reset this
								nurseHCPatternsShiftCounter = -1;
								isThisNurseInfoComplete = false;
								isStringInfoLine = true;
							}
							else if (tempWord == "Number")
							{
								isThisNurseInfoComplete = true;
								isStringInfoLine = true;
							}
						}
						else//is a number
						{
							if (tempWord.empty() == true)
							{
								//skip, do nothing
							}
							else
							{
								if (isStringInfoLine == true)
								{
									int tempInt = stoi(tempWord);

									if (isThisNurseInfoComplete == false)
									{
										if (currentIntNum == 0)
										{
											currentWeekNum = tempInt;
										}
										else if (currentIntNum == 1)
										{
											currentNurseNum = tempInt;
										}
										else
										{
											printf("some int reading error\n");
										}
									}
									else //isThisNurseInfoComplete == true
									{
										if (currentIntNum == 0)
										{
											numberOfPatterns = tempInt;

											nurseHCPatterns[currentWeekNum][currentNurseNum][nurseHCPatternsShiftCounter][ALLPOSSIBLECOMBINATIONS][0] = numberOfPatterns;
										}
										else
										{
											printf("some int reading error TYPE 2\n");
										}

									}

									currentIntNum++;
								}
								else//isStringInfoLine is false
								{
									int tempInt = stoi(tempWord);

									nurseHCPatterns[currentWeekNum][currentNurseNum][nurseHCPatternsShiftCounter][nurseHCPatternsCounter][currentIntNum] = tempInt;

									currentIntNum++;
								}
							}
						}

						//reset tempWord
						tempWord = "";
					}
				}

				if (currentIntNum >= WEEKNUMBEROFDAYS)
				{
					nurseHCPatternsCounter++;
				}

				if (isThisNurseInfoComplete == true)
				{
					nurseHCPatternsCounter = 0;
				}

				currentIntNum = 0;
				isStringInfoLine = false;
			}
		}
		else
		{
			printf("fileReadError\n");
		}
	}
	else
	{
		printf("fileNameError\n");
	}
}

//fileStreamNum 0 = patternDataFileStream
//fileStreamNum 1 = weeklyDataFileStream
void WriteLine(string line,int fileStreamNum)
{
	line.append("\n");

	if (fileStreamNum == 2)
	{
		if (weeklyDataFileStream.is_open())
		{
			weeklyDataFileStream << line;
			//cout << line;//testing purposes only
		}
		else
		{
			printf("Error opening file\n");
		}
	}
	else if (fileStreamNum == 1)
	{
		if (finalScheduleAnswerStream.is_open())
		{
			finalScheduleAnswerStream << line;
		}
		else
		{
			printf("Error opening file\n");
		}
	}
	else
	{
		printf("filestreamError\n");
	}
}

void CalculateWeekPatternCost(int currentShiftNum, int currentWeekNum, int arrayToBeCalculated[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][TOTALTYPEOFSHIFTS][WEEKNUMBEROFDAYS + 1])
{
	//check daily shift amount penalties
	int shiftNumber = 0;
	int shiftPenaltyCost = 0;
	int tempCoverDiff = 0;
	int dailyPenaltyValue = 0;

	int shiftAmounts[WEEKNUMBEROFDAYS] = { 0 };

	for (int i = 0; i < WEEKNUMBEROFDAYS; i++)
	{
		for (int j = 0; j < TOTALNUMBEROFNURSES; j++)
		{
			//allows for different shift acceptance
			shiftNumber = arrayToBeCalculated[currentWeekNum][j][currentShiftNum][i];

			if (shiftNumber == (currentShiftNum + 1))
			{
				shiftAmounts[i]++; //add to the particular shift amount
			}
		}
	}


	for (int j = 0; j < WEEKNUMBEROFDAYS; j++)
	{
		tempCoverDiff = shiftCoverAmount[currentShiftNum][0][j + (currentWeekNum * 7)] - shiftAmounts[j];

		if (tempCoverDiff >= 0)
		{
			dailyPenaltyValue = tempCoverDiff * shiftCoverAmount[currentShiftNum][1][j + (currentWeekNum * 7)];

			arrayToBeCalculated[currentWeekNum][TOTALNUMBEROFNURSES][currentShiftNum][j] = dailyPenaltyValue;

			shiftPenaltyCost += dailyPenaltyValue;
		}
		else //tempCoverDiff is negative (too many shifts assigned)
		{
			dailyPenaltyValue = (-tempCoverDiff) * shiftCoverAmount[currentShiftNum][2][j + (currentWeekNum * 7)];

			arrayToBeCalculated[currentWeekNum][TOTALNUMBEROFNURSES][currentShiftNum][j] = dailyPenaltyValue;

			shiftPenaltyCost += dailyPenaltyValue;
		}
	}
	
	//add shiftPenaltyCost to overall penalty costs
	arrayToBeCalculated[currentWeekNum][TOTALNUMBEROFNURSES][currentShiftNum][WEEKNUMBEROFDAYS] += shiftPenaltyCost;
}

void bubbleSort(int arr[TOTALNUMBEROFNURSES][TOTALTYPEOFSHIFTS][2], int currentNurseNum)
{
	bool swapped = true;
	int j = 0;
	int tmp1 = 0;
	int tmp2 = 0;

	while (swapped == true)
	{
		swapped = false;
		j++;

		for (int i = 0; i < TOTALTYPEOFSHIFTS - j; i++)
		{
			if (arr[currentNurseNum][i][1] < arr[currentNurseNum][i + 1][1])
			{
				tmp1 = arr[currentNurseNum][i + 1][0];
				tmp2 = arr[currentNurseNum][i + 1][1];

				arr[currentNurseNum][i + 1][0] = arr[currentNurseNum][i][0];
				arr[currentNurseNum][i + 1][1] = arr[currentNurseNum][i][1];

				arr[currentNurseNum][i][0] = tmp1;
				arr[currentNurseNum][i][1] = tmp2;
				swapped = true;
			}
		}
	}
}

bool CheckNurseCombinedShiftPatternValidity(int currentNurseNum, int currentWeekNum, int patternToBeChecked[TOTALNUMBEROFDAYS/WEEKNUMBEROFDAYS][WEEKNUMBEROFDAYS], bool isLastCalculatedShift)
{
	int totalNumberOfMinutesWorked = 0;

	int numberOfDaysWorkedByShift[TOTALTYPEOFSHIFTS] = { 0 };

	for (int k = 0; k < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; k++)
	{
		for (int l = 0; l < WEEKNUMBEROFDAYS; l++)
		{
			//check for all working days
			if (patternToBeChecked[k][l] != 0)
			{
				numberOfDaysWorkedByShift[patternToBeChecked[k][l] - 1]++;
			}
		}
	}

	//check for maximum number of shifts
	for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
	{
		if (numberOfDaysWorkedByShift[k] > maxShiftsPerType[currentNurseNum][k])
		{
			return false;
		}
	}


	//check for maximum number of minutes 
	for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
	{
		totalNumberOfMinutesWorked += numberOfDaysWorkedByShift[k] * shiftMinutesLengthInfo[k];
	}

	//check for minimum number of minutes only during the last week
	if (currentWeekNum >= (TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS) - 1 && isLastCalculatedShift == true)
	{
		if (totalNumberOfMinutesWorked < minMinutesPerNurse[currentNurseNum])
		{
			return false;
		}
	}

	if (totalNumberOfMinutesWorked > maxMinutesPerNurse[currentNurseNum])
	{
		return false;
	}

	totalNumberOfMinutesWorked = 0;
	/////////////////////////////////////////////////////////////////////////

	//check maximum consecutive shifts, minimum consecutive shifts and minimum consecutive days off
	int NurseConsecWorkingDays = 0;
	int NurseConsecDaysOff = 0;
	bool hasFirstWorkingDayStarted = false;

	for (int j = 0; j < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; j++)
	{
		for (int k = 0; k < WEEKNUMBEROFDAYS; k++)
		{
			int temp = patternToBeChecked[j][k];

			//is a working day shift
			if (temp != 0)
			{
				if ((NurseConsecWorkingDays >= maxConsecShift[currentNurseNum]) || (NurseConsecDaysOff < minConsecDaysOff[currentNurseNum] && NurseConsecDaysOff > 0))
				{
					NurseConsecWorkingDays = 0;
					NurseConsecDaysOff = 0;
					return false;
				}

				hasFirstWorkingDayStarted = true;
				NurseConsecWorkingDays++;
				NurseConsecDaysOff = 0;

			}
			else if (temp == 0)
			{
				if (NurseConsecWorkingDays < minConsecShift[currentNurseNum] && NurseConsecWorkingDays > 0)
				{
					NurseConsecWorkingDays = 0;
					NurseConsecDaysOff = 0;
					return false;
				}

				if (hasFirstWorkingDayStarted == true) //possibly only applicable to first week (must check)
				{
					NurseConsecDaysOff++;
				}

				NurseConsecWorkingDays = 0;

			}
			else
			{
				printf("ERROR\n");
			}
		}
	}

	NurseConsecWorkingDays = 0;
	NurseConsecDaysOff = 0;
	hasFirstWorkingDayStarted = false;

	///////////////////////////////////////////////////////////////
	//check maximum number of weekends
	int currentNumberOfWeekendsWorked = 0;

	for (int j = 0; j < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; j++)
	{
		//check saturday and sunday
		if ((patternToBeChecked[j][WEEKNUMBEROFDAYS - 2] != 0) || (patternToBeChecked[j][WEEKNUMBEROFDAYS - 1] != 0))
		{
			currentNumberOfWeekendsWorked++;
		}

		if (currentNumberOfWeekendsWorked > maxWeekends[currentNurseNum])
		{
			return false;
			break;
		}
	}

	currentNumberOfWeekendsWorked = 0;
	/////////////////////////////////////////////////////////////////////

	//works, return true
	return true;
}

bool MergedShiftPatternFilter(int currentWeekNum, int currentNurseNum, int currentShiftNum, int passedInAvailableDaySlots[WEEKNUMBEROFDAYS])
{
	for (int i = 0; i < WEEKNUMBEROFDAYS; i++)
	{
		if (tempMergeShiftSchedule[currentWeekNum][currentNurseNum][currentShiftNum][i] != 0)
		{
			if (passedInAvailableDaySlots[i] > 0)
			{
				for (int j = 0; j < nextShiftNotAllowedIntegerFormArray[passedInAvailableDaySlots[i]][TOTALTYPEOFSHIFTS]; j++)
				{
					if (currentShiftNum == nextShiftNotAllowedIntegerFormArray[passedInAvailableDaySlots[i] - 1][j])
					{
						return false;
					}
				}
			}
			else if (passedInAvailableDaySlots[i] < 0)// (means occupied day)
			{
				return false;
			}	
			else //availableDaySlots[i] == 0
			{
				/* not required
				for (int j = 0; j < nextShiftNotAllowedIntegerFormArray[currentShiftNum][TOTALTYPEOFSHIFTS]; j++)
				{
					if (i < WEEKNUMBEROFDAYS - 1)
					{
						if (passedInAvailableDaySlots[i + 1] == (nextShiftNotAllowedIntegerFormArray[currentShiftNum][j] + 1) * -1)
						{
							return false;
						}
					}
				}
				*/
			}
		}
	}

	return true;
}

void ChangePatternOneNurse(int currentWeekNum, int currentNurseNum)
{
	int tempDayPenaltyValue = 0;
	int penaltyCostEachOptionArray[ALLPOSSIBLECOMBINATIONS] = { 0 };

	int tempAvailableDaySlot[WEEKNUMBEROFDAYS] = { 0 };

	bool hasFirstPatternBeenAccepted = false;

	int minimumPenaltyCostValue = -1;
	int minimumPenaltyCostIndex = -1;

	//reset
	for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
	{
		for (int k = 0; k < WEEKNUMBEROFDAYS + 1; k++)
		{
			tempMergeShiftSchedule[currentWeekNum][currentNurseNum][j][k] = 0;
		}
	}

	//reset listOfAcceptedPatternsAllWeeks
	for (int i = 0; i < TOTALTYPEOFSHIFTS; i++)
	{
		for (int j = 0; j < ALLPOSSIBLECOMBINATIONS + 1; j++)
		{
			listOfAcceptedPatternsAllWeeks[currentWeekNum][currentNurseNum][i][j] = 0;
		}
	}

	for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
	{
		//reset all values
		minimumPenaltyCostValue = -1;
		minimumPenaltyCostIndex = -1;
		hasFirstPatternBeenAccepted = false;


		int currentShiftToBeChanged = allNurseShiftRankingAndPenaltyCost[currentNurseNum][j][0];

		// special condition if not first week
		if (currentWeekNum > 0)
		{
			for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
			{
				if (tempMergeShiftSchedule[currentWeekNum][currentShiftToBeChanged][k][WEEKNUMBEROFDAYS - 1] != 0)
				{
					if (nextShiftNotAllowedIntegerFormArray[k][TOTALTYPEOFSHIFTS] > 0) //this shift has hard constraint unavailable preceding shifts of current shift (variable) 
					{
						tempAvailableDaySlot[0] = k + 1;
						break;
					}
				}
			}
		}

		for (int k = 0; k < nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftToBeChanged][ALLPOSSIBLECOMBINATIONS][0]; k++)
		{
			bool isThisNewPattern = true;

			//if this is first shift
			if (j <= 0)
			{
				for (int m = 0; m < testedNurseFirstShiftScheduledSelectedPattern[currentWeekNum][currentNurseNum][ALLPOSSIBLECOMBINATIONS]; m++)
				{
					if (k == testedNurseFirstShiftScheduledSelectedPattern[currentWeekNum][currentNurseNum][m])
					{
						isThisNewPattern = false;
					}
				}
			}


			if (isThisNewPattern == true)
			{
				for (int m = 0; m < WEEKNUMBEROFDAYS + 1; m++)
				{
					tempMergeShiftSchedule[currentWeekNum][currentNurseNum][currentShiftToBeChanged][m] = nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftToBeChanged][k][m];
				}

				if (MergedShiftPatternFilter(currentWeekNum, currentNurseNum, currentShiftToBeChanged, tempAvailableDaySlot) == true)
				{
					int tempMergedArray[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][WEEKNUMBEROFDAYS] = { 0 };

					for (int p = 0; p < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; p++)
					{
						for (int m = 0; m < TOTALTYPEOFSHIFTS; m++)
						{
							for (int n = 0; n < WEEKNUMBEROFDAYS; n++)
							{
								if (tempMergedArray[p][n] == 0)
								{
									tempMergedArray[p][n] = tempMergeShiftSchedule[p][currentNurseNum][m][n];
								}
							}
						}
					}

					if (CheckNurseCombinedShiftPatternValidity(currentNurseNum, currentWeekNum, tempMergedArray, false) == true)
					{
						//count all the day penalty values
						for (int n = 0; n < TOTALNUMBEROFNURSES; n++)
						{
							tempDayPenaltyValue += tempMergeShiftSchedule[currentWeekNum][n][currentShiftToBeChanged][WEEKNUMBEROFDAYS];
						}

						tempMergeShiftSchedule[currentWeekNum][TOTALNUMBEROFNURSES][currentShiftToBeChanged][WEEKNUMBEROFDAYS] = tempDayPenaltyValue;

						CalculateWeekPatternCost(currentShiftToBeChanged, currentWeekNum, tempMergeShiftSchedule);
						penaltyCostEachOptionArray[k] = tempMergeShiftSchedule[currentWeekNum][TOTALNUMBEROFNURSES][currentShiftToBeChanged][WEEKNUMBEROFDAYS];

					}
					else
					{
						penaltyCostEachOptionArray[k] = -1;
					}


				}
				else //MergedShiftPatternFilter returns false
				{
					penaltyCostEachOptionArray[k] = -1;
				}

				tempDayPenaltyValue = 0;
			}
			else
			{
				penaltyCostEachOptionArray[k] = -10;
			}
		}

		//////////////////////////////////////////////////////////

		for (int k = 0; k < nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftToBeChanged][ALLPOSSIBLECOMBINATIONS][0]; k++)
		{
			if (k <= 0 || hasFirstPatternBeenAccepted == false)
			{
				//if this pattern is accepted
				if (penaltyCostEachOptionArray[k] >= 0)
				{
					minimumPenaltyCostValue = penaltyCostEachOptionArray[k];
					minimumPenaltyCostIndex = k;

					hasFirstPatternBeenAccepted = true;
				}
			}
			else
			{
				if (penaltyCostEachOptionArray[k] <= minimumPenaltyCostValue && penaltyCostEachOptionArray[k] >= 0)
				{
					minimumPenaltyCostValue = penaltyCostEachOptionArray[k];
					minimumPenaltyCostIndex = k;
				}
			}
		}

		//testing
		if (minimumPenaltyCostIndex == -1 || minimumPenaltyCostValue == -1)
		{
			printf("minimumPenaltyCost ERROR\n");
		}

		if (hasFirstPatternBeenAccepted == true)
		{
			int listOfAcceptedPatternsAllWeeksCounter = 0;

			if (j <= 0)
			{
				testedNurseFirstShiftScheduledSelectedPattern[currentWeekNum][currentNurseNum][testedNurseFirstShiftScheduledSelectedPattern[currentWeekNum][currentNurseNum][ALLPOSSIBLECOMBINATIONS]] = minimumPenaltyCostIndex;
				testedNurseFirstShiftScheduledSelectedPattern[currentWeekNum][currentNurseNum][ALLPOSSIBLECOMBINATIONS]++;
			}


			for (int k = 0; k < WEEKNUMBEROFDAYS + 1; k++)
			{
				tempMergeShiftSchedule[currentWeekNum][currentNurseNum][currentShiftToBeChanged][k] = nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftToBeChanged][minimumPenaltyCostIndex][k];
			}

			currentPatternSelectionAllWeeks[currentWeekNum][currentNurseNum][currentShiftToBeChanged] = minimumPenaltyCostIndex;

			for (int k = 0; k < nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftToBeChanged][ALLPOSSIBLECOMBINATIONS][0]; k++)
			{
				//if pattern is accepted
				if (penaltyCostEachOptionArray[k] >= 0)
				{
					listOfAcceptedPatternsAllWeeks[currentWeekNum][currentNurseNum][currentShiftToBeChanged][listOfAcceptedPatternsAllWeeksCounter] = k;
					listOfAcceptedPatternsAllWeeksCounter++;
				}
			}

			listOfAcceptedPatternsAllWeeks[currentWeekNum][currentNurseNum][currentShiftToBeChanged][ALLPOSSIBLECOMBINATIONS] = listOfAcceptedPatternsAllWeeksCounter;
		}
		else
		{
			printf("something wrong version 2 individual greedy search week: %d, nurse: %d, shift: %d\n", currentWeekNum, currentNurseNum, currentShiftToBeChanged);
		}

		//assigning availableDaySlots for merged array shift schedule
		for (int k = 0; k < WEEKNUMBEROFDAYS; k++)
		{
			if (tempMergeShiftSchedule[currentWeekNum][currentNurseNum][currentShiftToBeChanged][k] != 0)
			{
				tempAvailableDaySlot[k] = tempMergeShiftSchedule[currentWeekNum][currentNurseNum][currentShiftToBeChanged][k] * -1;
			}
			else if (nextShiftNotAllowedIntegerFormArray[currentShiftToBeChanged][TOTALTYPEOFSHIFTS] > 0) //this shift has hard constraint unavailable preceding shifts of current shift (variable) 
			{
				if (k > 0)
				{
					if (tempMergeShiftSchedule[currentWeekNum][currentNurseNum][currentShiftToBeChanged][k - 1] != 0)
					{
						tempAvailableDaySlot[k] = currentShiftToBeChanged + 1;
					}
				}
			}
		}

		//reset penaltyCostArray
		for (int k = 0; k < ALLPOSSIBLECOMBINATIONS; k++)
		{
			penaltyCostEachOptionArray[k] = 0;
		}
	}

}

bool GreedyMethodOneNurseOneShift(int currentWeekNum, int currentNurseNum, int currentShiftNum, int scheduledShiftNum, bool isLastCalculatedShift)
{
	int penaltyCostEachOptionArray[ALLPOSSIBLECOMBINATIONS] = { 0 };
	int minimumPenaltyCostValue = -1;
	int minimumPenaltyCostIndex = -1;

	int tempDayPenaltyValue = 0;

	bool hasFirstPatternBeenAccepted = false;

	bool isProcessFinished = false;


	while (isProcessFinished == false)
	{
		// special condition if not first week
		if (currentWeekNum > 0)
		{
			for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
			{
				if (tempMergeShiftSchedule[currentWeekNum - 1][currentNurseNum][k][WEEKNUMBEROFDAYS - 1] != 0)
				{
					if (nextShiftNotAllowedIntegerFormArray[k][TOTALTYPEOFSHIFTS] > 0) //this shift has hard constraint unavailable preceding shifts of current shift (variable) 
					{
						availableDaySlots[0] = k + 1;
						break;
					}
				}
			}
		}

		for (int k = 0; k < nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftNum][ALLPOSSIBLECOMBINATIONS][0]; k++)
		{
			for (int m = 0; m < WEEKNUMBEROFDAYS + 1; m++)
			{
				tempMergeShiftSchedule[currentWeekNum][currentNurseNum][currentShiftNum][m] = nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftNum][k][m];
			}

			if (MergedShiftPatternFilter(currentWeekNum, currentNurseNum, currentShiftNum, availableDaySlots) == true)
			{
				int tempMergedArray[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][WEEKNUMBEROFDAYS] = { 0 };

				for (int p = 0; p < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; p++)
				{
					for (int m = 0; m < TOTALTYPEOFSHIFTS; m++)
					{
						for (int n = 0; n < WEEKNUMBEROFDAYS; n++)
						{
							if (tempMergedArray[p][n] == 0)
							{
								tempMergedArray[p][n] = tempMergeShiftSchedule[p][currentNurseNum][m][n];
							}
						}
					}
				}


				if (CheckNurseCombinedShiftPatternValidity(currentNurseNum, currentWeekNum, tempMergedArray, isLastCalculatedShift) == true)
				{
					//count all the day penalty values
					for (int n = 0; n < TOTALNUMBEROFNURSES; n++)
					{
						tempDayPenaltyValue += tempMergeShiftSchedule[currentWeekNum][n][currentShiftNum][WEEKNUMBEROFDAYS];
					}

					tempMergeShiftSchedule[currentWeekNum][TOTALNUMBEROFNURSES][currentShiftNum][WEEKNUMBEROFDAYS] = tempDayPenaltyValue;

					CalculateWeekPatternCost(currentShiftNum, currentWeekNum, tempMergeShiftSchedule);
					penaltyCostEachOptionArray[k] = tempMergeShiftSchedule[currentWeekNum][TOTALNUMBEROFNURSES][currentShiftNum][WEEKNUMBEROFDAYS];

				}
				else
				{
					penaltyCostEachOptionArray[k] = -1;
				}


			}
			else //MergedShiftPatternFilter returns false
			{
				penaltyCostEachOptionArray[k] = -1;
			}

			tempDayPenaltyValue = 0;
		}

		//////////////////////////////////////////////////////////

		for (int k = 0; k < nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftNum][ALLPOSSIBLECOMBINATIONS][0]; k++)
		{
			if (k <= 0 || hasFirstPatternBeenAccepted == false)
			{
				//if this pattern is accepted
				if (penaltyCostEachOptionArray[k] >= 0)
				{
					minimumPenaltyCostValue = penaltyCostEachOptionArray[k];
					minimumPenaltyCostIndex = k;

					hasFirstPatternBeenAccepted = true;
				}
			}
			else
			{
				if (penaltyCostEachOptionArray[k] <= minimumPenaltyCostValue && penaltyCostEachOptionArray[k] >= 0)
				{
					minimumPenaltyCostValue = penaltyCostEachOptionArray[k];
					minimumPenaltyCostIndex = k;
				}
			}
		}

		if (hasFirstPatternBeenAccepted == true)
		{
			int listOfAcceptedPatternsAllWeeksCounter = 0;
			
			isProcessFinished = true;

			if (scheduledShiftNum <= 0)
			{
				testedNurseFirstShiftScheduledSelectedPattern[currentWeekNum][currentNurseNum][0] = minimumPenaltyCostIndex;
				testedNurseFirstShiftScheduledSelectedPattern[currentWeekNum][currentNurseNum][ALLPOSSIBLECOMBINATIONS]++;
			}

			for (int k = 0; k < WEEKNUMBEROFDAYS + 1; k++)
			{
				tempMergeShiftSchedule[currentWeekNum][currentNurseNum][currentShiftNum][k] = nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftNum][minimumPenaltyCostIndex][k];
			}

			currentPatternSelectionAllWeeks[currentWeekNum][currentNurseNum][currentShiftNum] = minimumPenaltyCostIndex;

			for (int k = 0; k < nurseHCPatterns[currentWeekNum][currentNurseNum][currentShiftNum][ALLPOSSIBLECOMBINATIONS][0]; k++)
			{
				//if pattern is accepted
				if (penaltyCostEachOptionArray[k] >= 0)
				{
					listOfAcceptedPatternsAllWeeks[currentWeekNum][currentNurseNum][currentShiftNum][listOfAcceptedPatternsAllWeeksCounter] = k;
					listOfAcceptedPatternsAllWeeksCounter++;
				}
			}

			listOfAcceptedPatternsAllWeeks[currentWeekNum][currentNurseNum][currentShiftNum][ALLPOSSIBLECOMBINATIONS] = listOfAcceptedPatternsAllWeeksCounter;
		}
		else
		{
			printf("individual greedy search week: %d, nurse: %d, shift: %d\n", currentWeekNum, currentNurseNum, currentShiftNum);

			//reset this nurse shift schedule
			for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
			{
				for (int m = 0; m < WEEKNUMBEROFDAYS + 1; m++)
				{
					tempMergeShiftSchedule[currentWeekNum][currentNurseNum][k][m] = 0;
					tempMergeShiftSchedule[currentWeekNum - 1][currentNurseNum][k][m] = 0;
				}
			}
			
			//reset
			for (int m = 0; m < WEEKNUMBEROFDAYS; m++)
			{
				availableDaySlots[m] = 0;
			}

			ChangePatternOneNurse(currentWeekNum - 1, currentNurseNum);

			return false;
		}
	}
	
	return true;
}

bool ShiftMerging(int currentWeekNum, bool isGreedySearch)
{	
	int nurseScheduleOrder[TOTALNUMBEROFNURSES][2] = { 0 };

	//copy finalScheduleOutput to tempMergeShiftSchedule
	for (int i = 0; i < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; i++)
	{
		for (int j = 0; j < TOTALNUMBEROFNURSES + 1; j++)
		{
			for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
			{
				for (int m = 0; m < WEEKNUMBEROFDAYS + 1; m++)
				{
					tempMergeShiftSchedule[i][j][k][m] = finalScheduleOutput[i][j][k][m];
				}
			}
		}
	}
	
	//sort nurse by maxMinutesPerNurse
	for (int i = 0; i < TOTALNUMBEROFNURSES; i++)
	{
		nurseScheduleOrder[i][0] = i;
		nurseScheduleOrder[i][1] = maxMinutesPerNurse[i];
	}

	//bubble sort by maxMinutesPerNurse length
	bool swapped = true;
	int jcounter = 0;
	int tmp1 = 0;
	int tmp2 = 0;

	while (swapped == true)
	{
		swapped = false;
		jcounter++;

		for (int i = 0; i < TOTALNUMBEROFNURSES - jcounter; i++)
		{
			if (nurseScheduleOrder[i][1] > nurseScheduleOrder[i + 1][1])
			{
				tmp1 = nurseScheduleOrder[i][0];
				tmp2 = nurseScheduleOrder[i][1];

				nurseScheduleOrder[i][0] = nurseScheduleOrder[i + 1][0];
				nurseScheduleOrder[i][1] = nurseScheduleOrder[i + 1][1];
				
				nurseScheduleOrder[i + 1][0] = tmp1;
				nurseScheduleOrder[i + 1][1] = tmp2;
				swapped = true;
			}
		}
	}

	for (int i = 0; i < TOTALNUMBEROFNURSES; i++)
	{
		//reset availableDaySlots
		for (int j = 0; j < WEEKNUMBEROFDAYS; j++)
		{
			availableDaySlots[j] = 0;
		}

		//calculate overall penalty cost
		for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
		{
			int tempDayPenaltyValue = 0;

			//count all the day penalty values
			for (int n = 0; n < TOTALNUMBEROFNURSES; n++)
			{
				tempDayPenaltyValue += tempMergeShiftSchedule[currentWeekNum][n][j][WEEKNUMBEROFDAYS];
			}

			tempMergeShiftSchedule[currentWeekNum][TOTALNUMBEROFNURSES][j][WEEKNUMBEROFDAYS] = tempDayPenaltyValue;

			CalculateWeekPatternCost(j, currentWeekNum, tempMergeShiftSchedule);
		}

		//compiling and sorting shift penalty costs based on highest nurse pattern penalty cost
		
		for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
		{
			allNurseShiftRankingAndPenaltyCost[i][j][0] = j;
			allNurseShiftRankingAndPenaltyCost[i][j][1] = nurseHCPatterns[currentWeekNum][nurseScheduleOrder[i][0]][j][nurseHCPatterns[currentWeekNum][nurseScheduleOrder[i][0]][j][ALLPOSSIBLECOMBINATIONS][0] - 1][WEEKNUMBEROFDAYS];
		}

		bubbleSort(allNurseShiftRankingAndPenaltyCost,i);

		//////////////////////////////////////////////////////

		for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
		{

			// special condition if not first week
			if (currentWeekNum > 0)
			{
				for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
				{
					if (tempMergeShiftSchedule[currentWeekNum - 1][nurseScheduleOrder[i][0]][k][WEEKNUMBEROFDAYS - 1] != 0)
					{
						if (nextShiftNotAllowedIntegerFormArray[k][TOTALTYPEOFSHIFTS] > 0) //this shift has hard constraint unavailable preceding shifts of current shift (variable) 
						{
							availableDaySlots[0] = k + 1;
							break;
						}
					}
				}
			}

			if (j >= TOTALTYPEOFSHIFTS - 1)
			{
				bool isLastShift = true;

				while (GreedyMethodOneNurseOneShift(currentWeekNum, nurseScheduleOrder[i][0], allNurseShiftRankingAndPenaltyCost[i][j][0], j, isLastShift) == false)
				{
					j = 0;
					isLastShift = false;
				}
			}
			else
			{
				while (GreedyMethodOneNurseOneShift(currentWeekNum, nurseScheduleOrder[i][0], allNurseShiftRankingAndPenaltyCost[i][j][0], j, false) == false)
				{
					j = 0;
				}
			}
			

			//assigning availableDaySlots for merged array shift schedule
			for (int k = 0; k < WEEKNUMBEROFDAYS; k++)
			{
				if (tempMergeShiftSchedule[currentWeekNum][nurseScheduleOrder[i][0]][allNurseShiftRankingAndPenaltyCost[i][j][0]][k] != 0)
				{
					availableDaySlots[k] = tempMergeShiftSchedule[currentWeekNum][nurseScheduleOrder[i][0]][allNurseShiftRankingAndPenaltyCost[i][j][0]][k] * -1;
				}
				else if (nextShiftNotAllowedIntegerFormArray[allNurseShiftRankingAndPenaltyCost[i][j][0]][TOTALTYPEOFSHIFTS] > 0) //this shift has hard constraint unavailable preceding shifts of current shift (variable) 
				{
					if (k > 0)
					{
						if (tempMergeShiftSchedule[currentWeekNum][nurseScheduleOrder[i][0]][allNurseShiftRankingAndPenaltyCost[i][j][0]][k - 1] != 0)
						{
							availableDaySlots[k] = allNurseShiftRankingAndPenaltyCost[i][j][0] + 1;
						}
					}
				}
			}

		}
	}

	//calculate all shifts overall penalty cost
	for (int k = 0; k < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; k++)
	{
		for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
		{
			int tempDayPenaltyValue = 0;

			//count all the day penalty values
			for (int n = 0; n < TOTALNUMBEROFNURSES; n++)
			{
				tempDayPenaltyValue += tempMergeShiftSchedule[k][n][j][WEEKNUMBEROFDAYS];
			}

			tempMergeShiftSchedule[k][TOTALNUMBEROFNURSES][j][WEEKNUMBEROFDAYS] = tempDayPenaltyValue;

			CalculateWeekPatternCost(j, k, tempMergeShiftSchedule);
		}
	}

	if (isGreedySearch == false)
	{
		//check hill climbing method
		bool isPatternSimilarToPreviousPattern = true;

		//if there are no changes in penalty cost, stop recursive hill climbing method
		for (int j = 0; j < TOTALTYPEOFSHIFTS; j++)
		{
			if (finalScheduleOutput[currentWeekNum][TOTALNUMBEROFNURSES][j][WEEKNUMBEROFDAYS] != tempMergeShiftSchedule[currentWeekNum][TOTALNUMBEROFNURSES][j][WEEKNUMBEROFDAYS])
			{
				isPatternSimilarToPreviousPattern = false;
			}
		}
		//if there are no changes in penalty cost, stop recursive hill climbing method
		if (isPatternSimilarToPreviousPattern == true)
		{
			return false;
		}
	}

	//copy tempMergeShiftSchedule to finalScheduleOutput
	for (int k = 0; k < TOTALNUMBEROFNURSES + 1; k++)
	{
		for (int m = 0; m < TOTALTYPEOFSHIFTS; m++)
		{
			for (int n = 0; n < WEEKNUMBEROFDAYS + 1; n++)
			{
				finalScheduleOutput[currentWeekNum][k][m][n] = tempMergeShiftSchedule[currentWeekNum][k][m][n];
			}
		}
	}
	
	return true;
}

int main(int argc, char * argv[])
{
	patternDataFileStream.open("patternDataInput.txt");
	finalScheduleAnswerStream.open("finalScheduleAnswerData.txt");
	scheduleConditionsStream.open("ScheduleInputConditions.txt");

	string outputLine = "";

	printf("make sure you changed number of nurses, shifts and schedule length before running new instances!\n");

	ReadFile(0);
	//copy data from file to nurseHCPatterns
	ReadFile(1);

	//create an integer array format for nextShiftNotAllowedArray
	for (int i = 0; i < TOTALTYPEOFSHIFTS; i++)
	{
		int nextShiftNotAllowedCounter = 0;
		string tempShiftID = "";

		tempShiftID = nextShiftNotAllowedInfo[i][nextShiftNotAllowedCounter];

		while (tempShiftID != "")
		{
			for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
			{
				if (shiftID[k] == tempShiftID)
				{
					nextShiftNotAllowedIntegerFormArray[i][nextShiftNotAllowedCounter] = k;
					break;
				}
			}

			nextShiftNotAllowedCounter++;
			tempShiftID = nextShiftNotAllowedInfo[i][nextShiftNotAllowedCounter];
		}
		
		//store array length at last index
		nextShiftNotAllowedIntegerFormArray[i][TOTALTYPEOFSHIFTS] = nextShiftNotAllowedCounter;
	}

	//program execution starts, everything before this is just reading input conditions data
	clock_t tStart = clock();

	//step 2 greedy search method
	for (int i = 0; i < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; i++)
	{
		//greedy method
		ShiftMerging(i,true);
	}

	//copy tempMergeShiftSchedule to finalScheduleOutput
	for (int i = 0; i < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; i++)
	{
		for (int k = 0; k < TOTALNUMBEROFNURSES + 1; k++)
		{
			for (int m = 0; m < TOTALTYPEOFSHIFTS; m++)
			{
				for (int n = 0; n < WEEKNUMBEROFDAYS + 1; n++)
				{
					finalScheduleOutput[i][k][m][n] = tempMergeShiftSchedule[i][k][m][n];
				}
			}
		}
	}
	
	//step 2b hill climbing method
	for (int i = 0; i < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; i++)
	{
		//hill climbing method
		while (ShiftMerging(i, false) == true)
		{
			printf("hill climbing recursive\n");
		}
	}
	
	//algorithm has finished running, everything after this is just printing to file
	printf("Time taken: %.4fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);

	//copy tempMergeShiftSchedule to finalScheduleOutput
	for (int i = 0; i < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; i++)
	{
		for (int k = 0; k < TOTALNUMBEROFNURSES + 1; k++)
		{
			for (int m = 0; m < TOTALTYPEOFSHIFTS; m++)
			{
				for (int n = 0; n < WEEKNUMBEROFDAYS + 1; n++)
				{
					finalScheduleOutput[i][k][m][n] = tempMergeShiftSchedule[i][k][m][n];
				}
			}
		}
	}


	//merge all shifts into final schedule
	for (int p = 0; p < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; p++)
	{
		int tempTotalDayPenalty = 0;

		for (int m = 0; m < TOTALNUMBEROFNURSES; m++)
		{
			for (int k = 0; k < TOTALTYPEOFSHIFTS; k++)
			{
				for (int n = 0; n < WEEKNUMBEROFDAYS; n++)
				{
					if (finalSchedule[p][m][n] == 0)
					{
						finalSchedule[p][m][n] = finalScheduleOutput[p][m][k][n];
					}
				}

				tempTotalDayPenalty += finalScheduleOutput[p][m][k][WEEKNUMBEROFDAYS];
			}

			finalSchedule[p][m][WEEKNUMBEROFDAYS] = tempTotalDayPenalty;

			tempTotalDayPenalty = 0;
		}

		for (int m = 0; m < TOTALTYPEOFSHIFTS; m++)
		{
			for (int n = 0; n < WEEKNUMBEROFDAYS + 1; n++)
			{
				finalSchedule[p][TOTALNUMBEROFNURSES][n] += finalScheduleOutput[p][TOTALNUMBEROFNURSES][m][n];
			}
		}	
	}

	//print greedy search new one
	for (int i = 0; i < TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS; i++)
	{
		outputLine = "GreedySearchNEW";
		WriteLine(outputLine, 1);

		outputLine = "Week " + to_string(i) + " Schedule";

		WriteLine(outputLine, 1);

		outputLine = "---------------------------------";
		WriteLine(outputLine, 1);
		outputLine = "";

		for (int j = 0; j < TOTALNUMBEROFNURSES + 1; j++)
		{
			if (j >= TOTALNUMBEROFNURSES)
			{
				outputLine = "++++++++++++++++++++++++++++";
				WriteLine(outputLine, 1);
				outputLine = "";
			}

			for (int k = 0; k < WEEKNUMBEROFDAYS; k++)
			{
				outputLine += to_string(finalSchedule[i][j][k]) + " ";
			}

			outputLine += "  " + to_string(finalSchedule[i][j][WEEKNUMBEROFDAYS]);
			WriteLine(outputLine, 1);

			outputLine = "";
		}

		WriteLine(outputLine, 1);
		WriteLine(outputLine, 1);
	}

	printf("Press enter key to quit\n");
	std::getchar();

	patternDataFileStream.close();
	weeklyDataFileStream.close();
	finalScheduleAnswerStream.close();

	return 0;
}
