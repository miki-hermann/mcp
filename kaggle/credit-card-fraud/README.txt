This directory contains the MCP treatment of the dataset from
https://www.kaggle.com/datasets/nelgiriyewithana/credit-card-fraud-detection-dataset-2023

This  dataset  contains  credit  card transactions  made  by  European
cardholders in the  year 2023. It comprises over  550,000 records, and
the   data   has  been   anonymized   to   protect  the   cardholders'
identities. The primary objective of this dataset is to facilitate the
development  of  fraud detection  algorithms  and  models to  identify
potentially fraudulent transactions.

The dataset contains the following attributes:
id: Unique identifier for each transaction
V1-V28: Anonymized features representing various transaction attributes (e.g., time, location, etc.)
Amount: The transaction amount
Class: Binary label indicating whether the transaction is fraudulent (FRAUDULENT) or not (NOT_fraudulent)

There are three possible tratments of the this dataset.
The default treatment is launched by "make" or "make 2023".
The treatment, where both FRAUDULENT and NOT_fraudulent parts of the sample are equal, is launched by "make equal".
The treatment with sampling after binarization is launched by "make after".
Only the "make equal" tratment contains a cleaning of the dataset.
It is a false cleaning just to ensure thet both FRAUDULENT and NOT_fraudulent parts will have equal cardinality.

The original dataset is compressed into a zip-archive. Each treatment starts with uncompressing that archive.

The file "*-overview.txt" contains the overview of the dataset with respect to the conecpt to learn.
The concept to learn is the Class attribute on column 30.

The results of each tratment can be found in the files
    *_FRAUDULENT_mat.out
    *_FRAUDULENT_uniq.out
    *_NOT_fraudulent_mat.out
    *_NOT_fraudulent_uniq.out
The files "*_mat.out" contain the result of treatment with repetitions,
the files "*_uniq.out"  contain the results without repetitions. 
The files "*_FRAUDULENT_*.out" contain the formula and the statistical results for FRAUDULENT use,
the files "*_NOT_fraudulent_*.out" contain the formula and the statistical results for non-fraudulent use.
