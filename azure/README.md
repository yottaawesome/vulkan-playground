# Azure Budget Configuration

This directory contains the Azure Bicep template for configuring subscription-level spend limits (budgets) in Azure.

## Files

- `budget.bicep` — Bicep template that defines an Azure Consumption Budget at the subscription scope.
- `budget.parameters.json` — Parameter file with default values for the budget deployment.

## Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `budgetName` | string | `monthly-spend-limit` | The name of the budget resource. |
| `amount` | int | `100` | The monthly spend limit in USD. |
| `timeGrain` | string | `Monthly` | Budget period: `Monthly`, `Quarterly`, or `Annually`. |
| `startDate` | string | `2025-01-01` | Budget start date (must be the first of a month). |
| `endDate` | string | `2026-12-31` | Budget end date. |
| `contactEmails` | array | `[]` | Email addresses to notify when thresholds are exceeded. |
| `firstThreshold` | int | `80` | Percentage of budget at which the first alert fires. |
| `secondThreshold` | int | `100` | Percentage of budget at which the second alert fires. |

## Deployment

To deploy the budget, you need the Azure CLI with Bicep support (`az bicep`) and appropriate permissions on the target subscription.

### Prerequisites

- [Azure CLI](https://learn.microsoft.com/en-us/cli/azure/install-azure-cli) installed and logged in (`az login`)
- Contributor or Cost Management Contributor role on the subscription

### Deploy with default parameters

```bash
az deployment sub create \
  --location eastus \
  --template-file azure/budget.bicep \
  --parameters azure/budget.parameters.json
```

### Deploy with custom values

```bash
az deployment sub create \
  --location eastus \
  --template-file azure/budget.bicep \
  --parameters amount=50 contactEmails='["you@example.com"]'
```

### Verify the budget

```bash
az consumption budget list --subscription <your-subscription-id>
```

## Notes

- Azure budgets are **informational only** — they send alert notifications but do **not** stop resource provisioning when the limit is reached.
- To enforce hard spend limits, consider using [Azure Policy](https://learn.microsoft.com/en-us/azure/governance/policy/overview) or per-resource quotas in addition to budgets.
