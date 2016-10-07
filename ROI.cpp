#include <iostream>
#include <math.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

// this function returns the expected wage (wich is dependent on anciennity)
// this is based on value from http://www.jobat.be/nl/artikels/wat-verdienen-ingenieurs/
//return     The expected wage at a certain age
int getBrutoWageMonthly(int age)
{
    int ageStartWorking = 22;
    int anciennity = age - ageStartWorking;
    if (anciennity < 0)
        anciennity = 0;

    int brutoWage=0;
    if (anciennity < 2)
        brutoWage = 2700;
    else if (anciennity < 5)
        brutoWage = 3000;
    else if (anciennity < 10)
        brutoWage = 3900;
    else if (anciennity < 20)
        brutoWage = 5200;
    else
        brutoWage = 6500;
    return brutoWage;
}

int getBrutoWageYearly(int age)
{
    return getBrutoWageMonthly(age)*12;
}

// you have to do some annoying things here
int getNettoWageYearly(int age)
{
    int brutoWage = 12*getBrutoWageMonthly(age);
    int taxes = (1 + 0.069) * ( (brutoWage * (1-0.102216)) * 0.3569 - (7090 * 0.25)  );
    int nettoWage = brutoWage - taxes;
    return nettoWage;
    /* from tests on taxcalc.be:
    seems like formula is more or less the following:
        totaleBelasing = (brutoInkomen - beroepsKosten) * basisbelasting - (belastingsvrijBedrag * korting)
        echteTotaleBelasing = (1+ gemeenteBelasting) * totaleBelasting
        Hier is (voor bruto 32400)
                gemeenteBelasting =     0.069 (voor Dilbeek)
                beroepsKosten           0.102216
                basisBelasting =        0.3569
                belastingsvrijBedrag =  7090
                korting =               0.25
        Komt dus neer op (1 + 0.069) * ( (32400 - 3311.8) * 0.3569 - (7090 * 0.25)
            Federale belastingen =      0.7401 * totaleBelasting
            Gewestelijke belasting =    0.2599 * totaleBelasting
            Gemeentelijke belasting =   0.069 * totaleBelasting
    */
}

int getNettoWageMonthly(int age)
{
    return getNettoWageYearly(age)/12.0;
}

//this function calculates the increase over a certain interval, using a change of 'percent' per period.
//param  change The change per period, in percent
//param  end     the last period
//param  start   the first period (default=0, so 'end' is the length of the interval)
//return         the modifier with which an amount has te be multiplied to reach the future value
float getFutureValue(float changePerPeriod, int end, int start=0)
{
    while (end<= start) {
        std::cout << "Please enter a proper value for end.";
        std::cin >> end;
    }
    return pow(1+changePerPeriod/100,(end - start));
}

//this function calculates the inflation-adjusted modifier
//param  inflation   the expected average intflation rate in %
//param  end         the last period
//param  start       the first period (default=0, so 'end' is the length of the interval)
//return             the modifier with which a start amount has to be multiplied to reach the future value
//formula from http://www.investopedia.com/terms/i/inflation_adjusted_return.asp
float getInflationModifier(float inflationRate, int end, int start=0)
{
    float onePeriod= (1+ inflationRate/100);
    return pow(onePeriod,(end - start));
}

//this function calculates the current value of an investement returnRate into account, over a period (end-start). It assumes all profits are reinvested.
//param returnRate, end, start    same as above
//param  periodicContribution     the contribution per period
// from http://www.moneychimp.com/articles/finworks/fmbasinv.htm
float getCumulativeValue(int startAmount, int periodicContribution,
                         float returnRate, int end, int start=0)
{
    returnRate = returnRate/100;
    float currentAmount = startAmount;
    for(int period = start; period<end; period++) {
        currentAmount = (currentAmount + periodicContribution) * (1+ returnRate);
    }
    return currentAmount;
}

// set min or max for the invested amount
// param    amount              The amount from which a part will be invested
//param     investedPercent     the percentage of amount invested
//param     minAmount   the minimum amount invested (only used if investedPercent returns something lower than this)
//param     maxAmount    the maximum amount invested (only used if investedPercent returns something higher than this)
int getInvestedAmountMonthly(int amount, float investPercentage, int minAmount, int maxAmount)
{
    float investedAmount = amount*investPercentage/100;
    if (investedAmount < minAmount)
        investedAmount = minAmount;
    else if (investedAmount > maxAmount)
        investedAmount = maxAmount;
    return investedAmount;
}

//this function calculates the current value of all money invested during work-part of life, taking inflation, average ROI, changing wages into account
float getFutureValueInvestedMoney(int startAge, int endAge, float investedPercent,
                            int minInvestedAmountMonthly, int maxInvestedAmountMonthly,
                            float returnRate, float inflationRate)
{
    float end_amount = 0;
    for (int age = startAge; age<endAge; age++) {
        //get wage for this year
        float nettoWageYearly = getNettoWageYearly(age);

        // *12 and /12 because it's easier to think in monthly wages or investments than yearly ones.
        int investedAmountYearly = 12*getInvestedAmountMonthly( nettoWageYearly/12, investedPercent,
                                   minInvestedAmountMonthly, maxInvestedAmountMonthly);

        // calculate the end value of this years' investment
        end_amount += getCumulativeValue(investedAmountYearly, 0, returnRate, endAge, age);

        DEBUG_MSG("------------ We're now at the age of " << age << "-------------------");
        DEBUG_MSG("Netto: " << int(nettoWageYearly));
        DEBUG_MSG("Bruto: " << int(getBrutoWageYearly(age)));
        DEBUG_MSG("Taxes: " << int(getBrutoWageYearly(age) - nettoWageYearly));
        DEBUG_MSG("Invested: " << investedAmountYearly);
        DEBUG_MSG("This year's investments returns: " << getCumulativeValue(investedAmountYearly,
                  0, returnRate, endAge, age));
        DEBUG_MSG("Current total: " << end_amount);
    }
    DEBUG_MSG("end amount: " << end_amount);
    return end_amount;
}

float getNowValueInvestedMoney(int startAge, int endAge, float investedPercent,
                               int minInvestedAmountMonthly, int maxInvestedAmountMonthly,
                               float returnRate, float inflationRate)
{
    float futureValue = getFutureValueInvestedMoney(startAge, endAge, investedPercent,
                        minInvestedAmountMonthly, maxInvestedAmountMonthly,
                        returnRate, inflationRate);
    // return the value in terms of current money (adjust for inflation)
    return futureValue/getInflationModifier(inflationRate, endAge, startAge);
}

int main()
{
    int startAge = 20;
    int endAge = 65;
    float investedPercent = 20;
    int minInvestedAmountMonthly = 500;
    int maxInvestedAmountMonthly = 2000;
    float returnRate = 6.0;
    float inflationRate = 2.0;
    float value = getNowValueInvestedMoney(startAge, endAge, investedPercent,
                        minInvestedAmountMonthly, maxInvestedAmountMonthly,
                        returnRate, inflationRate);
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "The total value (in money at present) is: " << value << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
}



