targetScope = 'subscription'

@description('The name of the budget.')
param budgetName string = 'monthly-spend-limit'

@description('The total amount of cost or usage to track with the budget.')
param amount int = 100

@description('The time grain of the budget. Accepted values are Monthly, Quarterly, Annually.')
@allowed([
  'Monthly'
  'Quarterly'
  'Annually'
])
param timeGrain string = 'Monthly'

@description('The start date for the budget. Must be first of month in YYYY-MM-DD format.')
param startDate string = '2025-01-01'

@description('The end date for the budget in YYYY-MM-DD format.')
param endDate string = '2026-12-31'

@description('The list of email addresses to notify when the threshold is exceeded.')
param contactEmails array = []

@description('Threshold percentage at which the first alert notification is sent.')
param firstThreshold int = 80

@description('Threshold percentage at which the second alert notification is sent.')
param secondThreshold int = 100

resource budget 'Microsoft.Consumption/budgets@2021-10-01' = {
  name: budgetName
  properties: {
    timePeriod: {
      startDate: startDate
      endDate: endDate
    }
    timeGrain: timeGrain
    amount: amount
    category: 'Cost'
    notifications: {
      notificationForExceededBudget1: {
        enabled: true
        operator: 'GreaterThan'
        threshold: firstThreshold
        contactEmails: contactEmails
      }
      notificationForExceededBudget2: {
        enabled: true
        operator: 'GreaterThan'
        threshold: secondThreshold
        contactEmails: contactEmails
      }
    }
  }
}

output budgetName string = budget.name
output budgetAmount int = amount
