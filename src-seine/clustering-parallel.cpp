void clustering(Matrix &batch) {
  Row cl_flag (batch[0].size(), false);
  string new_varid = "z";

  outfile << "+++ Original arity = " << arity << endl;
  outfile << "+++ Clustering with epsilon = "
	  << to_string(cluster)
	  << endl;

  Matrix tr_batch = transpose(batch);
  Matrix new_tr;
  queue<int> cl_q;
  int cl_pointer = 0;
  int number_of_clusters = 0;

  if (varswitch)
    outfile << "+++ Own variable names canceled by clustering" << endl;
  outfile << "+++ Correspondence table with centers and their distance from average" << endl;
  
  for (int i = 0; i < cl_flag.size(); ++i)
    if (! cl_flag[i]) {
      cl_q.push(i);
      cl_flag[i] = true;
      vector<int> cl_bag;
      cl_bag.push_back(i);

      vector<int> center;
      for (int j = 0; j < tr_batch[i].size(); ++j)
	center.push_back(tr_batch[i][j]);
      int cl_card = 1;

      while (! cl_q.empty()) {
	int index = cl_q.front();
	cl_q.pop();

	for (int j = i+1; j < cl_flag.size(); ++j)
	  if (!cl_flag[j]
	      &&
	      (hamming_distance(tr_batch[j], tr_batch[index]) <= cluster)) {
	    cl_q.push(j);
	    cl_flag[j] = true;
	    cl_card++;
	    cl_bag.push_back(j);
	    for (int k = 0; k < tr_batch[i].size(); ++k)
	      center[k] += tr_batch[j][k];
	  }
      }
      
      Row false_center;
      for (int j = 0; j < center.size(); ++j)
	false_center.push_back(center[j] / cl_card + 0.5 < 1.0 ? false : true);
      int hdistance = tr_batch.size()+1;
      int cindex;
      Row true_center = false_center;
      for (int j : cl_bag) {
	int hd = hamming_distance(false_center, tr_batch[j]);
	if (hd < hdistance) {
	  hdistance = hd;
	  true_center = tr_batch[j];
	  cindex = j;
	}
      }
      new_tr.push_back(true_center);

      sort(cl_bag.begin(), cl_bag.end());
      outfile << "\t{ ";
      int cl_ctr= 0;
      for (int num : cl_bag) {
	if (++cl_ctr % CLUSTERLIMIT == 0)
	  outfile << endl << "\t  ";
	if (varswitch)
	  outfile << varnames[num] + " ";
	else
	  outfile << varid + to_string(offset + num) + " ";
      }
      outfile << "} ["
	      << to_string(cl_card)
	      << "] ";
      if (cl_bag.size() > 1)
	outfile << "\n\t\t";
      outfile << "\t-> " << new_varid << to_string(offset + number_of_clusters++);
      if (cl_bag.size() > 1) {
	vector<int> ham_dist;
	float mean_dist = 0.0;
	for (int j : cl_bag) {
	  int hd = hamming_distance(true_center, tr_batch[j]);
	  ham_dist.push_back(hd);
	  mean_dist += hd;
	}
	mean_dist /= cl_card;

	float std_dev = 0.0;
	for (int hd : ham_dist)
	  std_dev += (mean_dist - hd) * (mean_dist - hd);
	std_dev = sqrt(std_dev / cl_card);
	
	outfile << "\t(center = "
		<< (varswitch ? varnames[cindex] : varid + to_string(cindex))
		<< ", delta = " << hdistance
		<< ", mean distance = " << mean_dist
		<< ", std.dev. = " << std_dev
		<< ")";
      }
      outfile << endl;
    }

  outfile << "+++ Number of clusters = "
	  << number_of_clusters
	  << endl;

  batch = transpose(new_tr);
  arity = batch[0].size();
  varid = new_varid;
  varswitch = false;
}
