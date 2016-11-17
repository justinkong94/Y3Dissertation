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

const int SELFCONSTRAINTMINWORKINGDAYSAWEEK = 0;

const int ALLPOSSIBLECOMBINATIONS = 128; // for 2 shifts 2 power 7 = 128

const int MAXWEEKLYSCPENALTYCOST = 700;
const int MINWEEKLYSCPENALTYCOST = 0;
const int MAXFINALPENALTYCOST = 1000;

const int INDIVIDUALNURSEPATTERNPENALTYCOSTMININDEX = 0; //min index
const int INDIVIDUALNURSEPATTERNPENALTYCOSTMAXINDEX = 3000; //range of penalty costs increase available from zero cost
const bool ISNURSEPATTERNRANGEADJUSTED = false;

const int MAXNUMBEROFWEEKLYPATTERNSGENERATED = 10000;

const int WEEKNUMBEROFDAYS = 7;

string shiftID[TOTALTYPEOFSHIFTS];
int shiftMinutesLengthInfo[TOTALTYPEOFSHIFTS] = { 0 };
string nextShiftNotAllowedInfo[TOTALTYPEOFSHIFTS][TOTALTYPEOFSHIFTS];

int maxConsecShift[TOTALNUMBEROFNURSES] = { 0 };
int minConsecShift[TOTALNUMBEROFNURSES] = { 0 };

int minConsecDaysOff[TOTALNUMBEROFNURSES] = { 0 };

int maxWeekends[TOTALNUMBEROFNURSES] = { 0 };

string nurseID[TOTALNUMBEROFNURSES];

//MaxShifts
int maxShiftsPerType[TOTALNUMBEROFNURSES][TOTALTYPEOFSHIFTS] = { 0 };

int maxMinutesPerNurse[TOTALNUMBEROFNURSES] = { 0 };
int minMinutesPerNurse[TOTALNUMBEROFNURSES] = { 0 };


int shiftHCDaysOff[TOTALNUMBEROFNURSES][TOTALNUMBEROFDAYS] = { 0 }; // -1 if no days off

int NumberOfShiftsInADay[2][WEEKNUMBEROFDAYS] = { 0 }; //index 0 is for total number of shifts, index 1 is for comparison between section cover request

int NurseAcceptedCoverCombinations[TOTALNUMBEROFNURSES][ALLPOSSIBLECOMBINATIONS] = { 0 }; //contains the index of accepted combinations in FinalCombinationsArray

int FinalCombinationsArray[TOTALTYPEOFSHIFTS][ALLPOSSIBLECOMBINATIONS + 1][WEEKNUMBEROFDAYS] = { 0 }; // all possible combinations
int FinalCombinationsArrayReverse[TOTALTYPEOFSHIFTS][ALLPOSSIBLECOMBINATIONS + 1][WEEKNUMBEROFDAYS] = { 0 };

//index 0 is shift cover amount
//index 1 is shift penalty weight for under supply
//index 2 is shift penalty weight for over supply
int shiftCoverAmount[TOTALTYPEOFSHIFTS][3][TOTALNUMBEROFDAYS] = { 0 };

//this is priority on days to work
//numbers in array are weights in day order (higher weight, higher priority)
int ShiftOnRequest[TOTALTYPEOFSHIFTS][TOTALNUMBEROFNURSES][TOTALNUMBEROFDAYS] = { 0 };

//this is priority on rest days
//numbers in array are weights in day order (higher weight, higher priority)
int ShiftOffRequest[TOTALTYPEOFSHIFTS][TOTALNUMBEROFNURSES][TOTALNUMBEROFDAYS] = { 0 };

////////////////new code

//WEEKNUMBEROFDAYS starts from index 0, last extra index is used for penalty cost value
//ALLPOSSIBLECOMBINATIONS + 1 stores the actual number of occupied array indexes
//ALLPOSSIBLECOMBINATIONS - 1 index stores the starting index of array
int nurseHCPatterns[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES][TOTALTYPEOFSHIFTS][ALLPOSSIBLECOMBINATIONS + 1][WEEKNUMBEROFDAYS + 1] = { 0 };

//TOTALNUMBEROFNURSES + 1, extra array slot is used to store pattern penalty costs at (index 0 of 3rd dimension)
int allCostsFullWeekFinalPatternsArray[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][MAXNUMBEROFWEEKLYPATTERNSGENERATED][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };
int allCostsArrayCounter = 0;

int allAcceptedFinalScheduleArray[MAXNUMBEROFWEEKLYPATTERNSGENERATED][TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };
int allAcceptedFinalScheduleArrayCounter = 0;

int tempScheduleOutput[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };

int allWeeksAcceptedPatternIndex[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES][MAXNUMBEROFWEEKLYPATTERNSGENERATED + 1] = { 0 };

//new items
int greedySearchPreliminarySchedule[TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS][TOTALNUMBEROFNURSES + 1][WEEKNUMBEROFDAYS + 1] = { 0 };



fstream patternDataFileStream;
fstream weeklyDataFileStream;
fstream finalScheduleAnswerStream;
ifstream scheduleConditionsStream;

bool isNumeric(const std::string& input) {
	return std::all_of(input.begin(), input.end(), ::isdigit);
}

void ReadFile(int fileStreamNum)
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

	if (fileStreamNum == 0)
	{
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

									/*
									//if this is second last character of line
									if (i >= tempLine.size() - 2)
									{
										shiftTypeArrayIndex++;
									}
									*/
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
		else //wrong fileStreamNum
		{
			printf("Wrong FileStreamNum Error\n");
		}
	}
}


//fileStreamNum 0 = patternDataFileStream
//fileStreamNum 1 = weeklyDataFileStream
void WriteLine(string line, int fileStreamNum)
{
	line.append("\n");

	if (fileStreamNum == 0)
	{
		if (patternDataFileStream.is_open())
		{
			patternDataFileStream << line;
			//cout << line;//testing purposes only
		}
		else
		{
			printf("Error opening file\n");
		}
	}
	else if (fileStreamNum == 1)
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
	else if (fileStreamNum == 2)
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

//Hard constraints check
bool CheckPattern(int currentShiftType, int combinationArrayIndex, int currentNurseNum, int currentWeekNum)
{
	int NurseConsecWorkingDays = 0;
	int NurseConsecDaysOff = 0;

	int currentMaxShiftsAmount = 0;
	int numberOfShiftsThisWeek = 0;

	int currentShiftTypeMinutesLength = 0;
	int currentWeekShiftMinutesLength = 0;

	bool hasFirstWorkingDayStarted = false;

	for (int j = 0; j < WEEKNUMBEROFDAYS; j++)
	{
		int temp = FinalCombinationsArray[currentShiftType][combinationArrayIndex][j];

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
			
			//this is only checked if the TOTALTYPEOFSHIFTS is at most 1
			if (NurseConsecWorkingDays < minConsecShift[currentNurseNum] && NurseConsecWorkingDays > 0 && TOTALTYPEOFSHIFTS <= 1)
			{
				if ((currentWeekNum < (TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS) - 1 && j == 6) || (currentWeekNum <= 0 && j == 0))
				{
					//break, this sequence is accepted
				}
				else
				{
					//if this is not first week and is second day of the week
					if (currentWeekNum > 0 && j == 1)
					{
						//accepted, do nothing
					}
					else
					{
						NurseConsecWorkingDays = 0;
						NurseConsecDaysOff = 0;
						return false;
					}
				}	
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

	NurseConsecWorkingDays = 0;
	NurseConsecDaysOff = 0;


	//check for MaxShifts violation
	currentMaxShiftsAmount = maxShiftsPerType[currentNurseNum][currentShiftType];

	for (int j = 0; j < WEEKNUMBEROFDAYS; j++)
	{
		if (FinalCombinationsArray[currentShiftType][combinationArrayIndex][j] == (currentShiftType + 1))
		{
			numberOfShiftsThisWeek++;
		}
	}

	if (numberOfShiftsThisWeek > currentMaxShiftsAmount)
	{
		return false;
	}


	//check for MaxTotalMinutes violation
	currentShiftTypeMinutesLength = shiftMinutesLengthInfo[currentShiftType];

	currentWeekShiftMinutesLength = currentShiftTypeMinutesLength * numberOfShiftsThisWeek;

	if (currentWeekShiftMinutesLength > maxMinutesPerNurse[currentNurseNum])
	{
		return false;
	}

	return true;
}

//soft constraints check
bool CheckPatternFilter(int currentShiftType, int combinationArrayIndex, int currentNurseNum, int currentWeekNum)
{
	int nurseNumDaysWorked = 0;

	for (int j = 0; j < WEEKNUMBEROFDAYS; j++)
	{
		if (FinalCombinationsArray[currentShiftType][combinationArrayIndex][j] != 0)
		{
			nurseNumDaysWorked++;
		}
	}

	if (nurseNumDaysWorked < SELFCONSTRAINTMINWORKINGDAYSAWEEK)
	{
		return false;
	}

	/* does not always produce better results, therefore I have left this out for the final submission

	//weekend filter
	int saturdayShiftAmount = shiftCoverAmount[currentShiftType][0][(currentWeekNum * 7) + 5];
	int sundayShiftAmount = shiftCoverAmount[currentShiftType][0][(currentWeekNum * 7) + 6];

	if (saturdayShiftAmount > sundayShiftAmount)
	{
		//if saturday does not have this shift and sunday has this shift, return false
		if ((FinalCombinationsArray[currentShiftType][combinationArrayIndex][5] == 0) && (FinalCombinationsArray[currentShiftType][combinationArrayIndex][6] == (currentShiftType + 1)))
		{
			return false;
		}
	}
	else if (sundayShiftAmount > saturdayShiftAmount)
	{
		//if saturday has this shift and sunday does not have this shift, return false
		if ((FinalCombinationsArray[currentShiftType][combinationArrayIndex][5] == (currentShiftType + 1)) && (FinalCombinationsArray[currentShiftType][combinationArrayIndex][6] == 0))
		{
			return false;
		}
	}
	else if (saturdayShiftAmount == sundayShiftAmount)
	{
		//if saturday and sunday does not have the same shift type, return false
		if ((FinalCombinationsArray[currentShiftType][combinationArrayIndex][5] == (currentShiftType + 1) && (FinalCombinationsArray[currentShiftType][combinationArrayIndex][6] == 0))
			|| (FinalCombinationsArray[currentShiftType][combinationArrayIndex][5] == 0 && (FinalCombinationsArray[currentShiftType][combinationArrayIndex][6] == (currentShiftType + 1)))
			)
		{
			return false;
		}
	}
	else
	{
		printf("weekends shift amount error\n");
	}
	*/

	//all conditions accepted, return true
	return true;
}

//sort nurse pattern by penalty cost and also changes/limits the maximum number of patterns in this set available for scheduling
void NursePatternHCSort(int currentShiftType, int currentWeekNum)
{
	int highestPenaltyValue = 0;
	int highestPenaltyValuePatternIndex = 0;

	for (int i = 0; i < TOTALNUMBEROFNURSES; i++)
	{
		int tempHCSortArray[ALLPOSSIBLECOMBINATIONS][WEEKNUMBEROFDAYS + 1] = { 0 };

		//sorting by penalty cost
		for (int j = nurseHCPatterns[currentWeekNum][i][currentShiftType][ALLPOSSIBLECOMBINATIONS][0] - 1; j >= 0; j--)
		{
			for (int k = 0; k < nurseHCPatterns[currentWeekNum][i][currentShiftType][ALLPOSSIBLECOMBINATIONS][0]; k++)
			{
				if (nurseHCPatterns[currentWeekNum][i][currentShiftType][k][WEEKNUMBEROFDAYS] >= highestPenaltyValue)
				{
					highestPenaltyValue = nurseHCPatterns[currentWeekNum][i][currentShiftType][k][WEEKNUMBEROFDAYS];
					highestPenaltyValuePatternIndex = k;
				}
			}

			//copy the target index over
			for (int k = 0; k < WEEKNUMBEROFDAYS + 1; k++)
			{
				tempHCSortArray[j][k] = nurseHCPatterns[currentWeekNum][i][currentShiftType][highestPenaltyValuePatternIndex][k];
			}

			//delete that index from nurseHCPatterns
			for (int k = 0; k < WEEKNUMBEROFDAYS + 1; k++)
			{
				nurseHCPatterns[currentWeekNum][i][currentShiftType][highestPenaltyValuePatternIndex][k] = -1;
			}

			highestPenaltyValue = 0;
			highestPenaltyValuePatternIndex = 0;
		}

		//copy completed sorted array back to nurseHCPatterns
		for (int j = 0; j < nurseHCPatterns[currentWeekNum][i][currentShiftType][ALLPOSSIBLECOMBINATIONS][0]; j++)
		{
			//copy the target index over
			for (int k = 0; k < WEEKNUMBEROFDAYS + 1; k++)
			{
				nurseHCPatterns[currentWeekNum][i][currentShiftType][j][k] = tempHCSortArray[j][k];
			}
		}
	}
}

void CalculateDayOnAndOffPenaltyCost(int currentShiftType, int currentWeekNum)
{
	int totalPenaltyValue = 0;

	for (int i = 0; i < TOTALNUMBEROFNURSES; i++)
	{
		for (int j = 0; j < nurseHCPatterns[currentWeekNum][i][currentShiftType][ALLPOSSIBLECOMBINATIONS][0]; j++)
		{
			for (int l = 0; l < WEEKNUMBEROFDAYS; l++)
			{
				if (ShiftOnRequest[currentShiftType][i][l + (currentWeekNum * 7)] != 0 && (nurseHCPatterns[currentWeekNum][i][currentShiftType][j][l] != currentShiftType + 1))// if a penalty value is set for this day and the shift is not assigned on that day for this pattern
				{
					totalPenaltyValue += ShiftOnRequest[currentShiftType][i][l + (currentWeekNum * 7)];
				}

				if (ShiftOffRequest[currentShiftType][i][l + (currentWeekNum * 7)] != 0 && (nurseHCPatterns[currentWeekNum][i][currentShiftType][j][l] == currentShiftType + 1))// if a penalty value is set for this day and the shift is assigned on that day for this pattern
				{
					totalPenaltyValue += ShiftOffRequest[currentShiftType][i][l + (currentWeekNum * 7)];
				}	
			}

			nurseHCPatterns[currentWeekNum][i][currentShiftType][j][WEEKNUMBEROFDAYS] = totalPenaltyValue;
			totalPenaltyValue = 0;
		}
	}
}

int main(int argc, char * argv[])
{
	int combinationTypes[] = { 0, 0, 0, 0, 0, 0, 0 }; // set first value in array to 00000000

	int pArrayLength = 1; // used to count

	patternDataFileStream.open("PatternDataOutput.txt");
	scheduleConditionsStream.open("ScheduleInputConditions.txt");

	printf("make sure you changed number of nurses, shifts and schedule length before running new instances!\n");

	ReadFile(0);

	//program execution starts, everything before this is just reading input conditions data
	clock_t tStart = clock();

	for (int h = 0; h < TOTALTYPEOFSHIFTS; h++)
	{
		//set the second value in array to 11111111 or 2222222 etc
		for (int i = 0; i < 7; i++)
		{
			FinalCombinationsArray[h][pArrayLength][i] = { h + 1 };
		}

		pArrayLength++;


		// produce all possible combinations
		for (int i = 6; i > 0; i--)
		{
			combinationTypes[i] = h + 1;

			do {
				for (int j = 0; j < 7; j++)
				{
					FinalCombinationsArray[h][pArrayLength][j] = combinationTypes[j];
				}

				pArrayLength++;

			} while (std::next_permutation(combinationTypes, combinationTypes + WEEKNUMBEROFDAYS));
		}

		for (int i = 0; i < WEEKNUMBEROFDAYS; i++)
		{
			combinationTypes[i] = 0;
		}

		FinalCombinationsArray[h][ALLPOSSIBLECOMBINATIONS][0] = pArrayLength; //store array length at the end

		pArrayLength = 1; //reset
	}


	////////////////////////////////////
	//reverse array
	for (int h = 0; h < TOTALTYPEOFSHIFTS; h++)
	{
		for (int i = 0; i < FinalCombinationsArray[h][ALLPOSSIBLECOMBINATIONS][0]; i++)
		{
			for (int j = 0; j < WEEKNUMBEROFDAYS; j++)
			{
				FinalCombinationsArrayReverse[h][i][j] = FinalCombinationsArray[h][FinalCombinationsArray[h][ALLPOSSIBLECOMBINATIONS][0] - 1 - i][j];
			}
		}

		for (int i = 0; i < FinalCombinationsArray[h][ALLPOSSIBLECOMBINATIONS][0]; i++)
		{
			for (int j = 0; j < WEEKNUMBEROFDAYS; j++)
			{
				FinalCombinationsArray[h][i][j] = FinalCombinationsArrayReverse[h][i][j];
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////
	//CUTOFF FROM OLD CODE STEP 1

	for (int currentWeekNum = 0; currentWeekNum < (TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS); currentWeekNum++)
	{
		//checking for dayOff hard constraint, removing patterns which violate these and sort into individual nurses
		for (int i = 0; i < TOTALNUMBEROFNURSES; i++)
		{
			int HCDayOffCurrentWeekArray[WEEKNUMBEROFDAYS] = { 0 };
			int HCDaysOffCurrentWeekArrayCounter = 0;
			int tempHCDayOffValue = 0;
			int arrayCounter = 0;

			bool isCombinationAcceptedNoHCViolation = true;

			for (int m = 0; m < shiftHCDaysOff[i][TOTALNUMBEROFDAYS - 1]; m++)
			{
				tempHCDayOffValue = shiftHCDaysOff[i][m];

				tempHCDayOffValue -= (currentWeekNum * 7);

				//if HCDayOff is this week, add to HCDayOffCurrentWeekArray
				if (tempHCDayOffValue >= 0 && tempHCDayOffValue < WEEKNUMBEROFDAYS)
				{
					HCDayOffCurrentWeekArray[HCDaysOffCurrentWeekArrayCounter] = tempHCDayOffValue;
					HCDaysOffCurrentWeekArrayCounter++;
				}
			}


			for (int h = 0; h < TOTALTYPEOFSHIFTS; h++)
			{	
				for (int j = 0; j < FinalCombinationsArray[h][ALLPOSSIBLECOMBINATIONS][0]; j++)
				{
					if (CheckPattern(h, j, i, currentWeekNum) == true && CheckPatternFilter(h, j, i, currentWeekNum) == true)
					{
						for (int m = 0; m < HCDaysOffCurrentWeekArrayCounter; m++)
						{
							//if accepted combination has any HCDayOffViolation, reject combination
							if (FinalCombinationsArray[h][j][HCDayOffCurrentWeekArray[m]] != 0)
							{
								isCombinationAcceptedNoHCViolation = false;
								break;
							}
						}

						if (isCombinationAcceptedNoHCViolation == true)
						{
							for (int k = 0; k < WEEKNUMBEROFDAYS; k++)//starts from index 0, last extra index is used for penalty cost value
							{
								nurseHCPatterns[currentWeekNum][i][h][arrayCounter][k] = FinalCombinationsArray[h][j][k];
							}

							arrayCounter++;
						}
					}

					isCombinationAcceptedNoHCViolation = true;
				}
				
				nurseHCPatterns[currentWeekNum][i][h][ALLPOSSIBLECOMBINATIONS][0] = arrayCounter;
				arrayCounter = 0;
			}
		}
	}

	//calculate penalty costs and sort
	for (int currentWeekNum = 0; currentWeekNum < (TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS); currentWeekNum++)
	{
		for (int h = 0; h < TOTALTYPEOFSHIFTS; h++)
		{
			CalculateDayOnAndOffPenaltyCost(h, currentWeekNum);
			//sort patterns bsed on penalty cost
			NursePatternHCSort(h, currentWeekNum);
		}
	}

	//algorithm has finished running, everything after this is just printing to file
	printf("Time taken: %.4fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);

	//step 2c print patterns to file
	for (int currentWeekNum = 0; currentWeekNum < (TOTALNUMBEROFDAYS / WEEKNUMBEROFDAYS); currentWeekNum++)
	{
		//print patterns to file
		for (int i = 0; i < TOTALNUMBEROFNURSES; i++)
		{
			int nursePatternLength = 0;

			//print info
			string line = "Week " + to_string(currentWeekNum) + " Nurse " + to_string(i);
			WriteLine(line, 0);
			line = "---------------------------";
			WriteLine(line, 0);
			line = "";//empty string

			for (int h = 0; h < TOTALTYPEOFSHIFTS; h++)
			{
				int nurseHCArrayLength = nurseHCPatterns[currentWeekNum][i][h][ALLPOSSIBLECOMBINATIONS][0];
				int nurseHCArrayStartIndex = nurseHCPatterns[currentWeekNum][i][h][ALLPOSSIBLECOMBINATIONS - 1][0];

				for (int j = nurseHCArrayStartIndex; j < nurseHCArrayLength; j++)
				{
					for (int k = 0; k < WEEKNUMBEROFDAYS; k++)//starts from index 0, last extra index is used for penalty cost value
					{
						line += to_string(nurseHCPatterns[currentWeekNum][i][h][j][k]) + " ";
					}

					line += "  " + to_string(nurseHCPatterns[currentWeekNum][i][h][j][WEEKNUMBEROFDAYS]);//add penalty costs

					WriteLine(line, 0);
					line = "";//empty string
				}

				nursePatternLength += nurseHCArrayLength - nurseHCArrayStartIndex;
				line = "++++++++++++++++++++++++";
				WriteLine(line, 0);
				line = "";
			}

			WriteLine(line, 0);//print a blank line
			line = "Number of patterns: " + to_string(nursePatternLength);
			WriteLine(line, 0);//print arrayCounter

			line = "";
			WriteLine(line, 0);//print blank line8
			WriteLine(line, 0);//print blank line
		}
	}

	printf("Press enter key to quit\n");
	std::getchar();

	return 0;
}