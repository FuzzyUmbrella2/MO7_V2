#ifndef COEFFICIEMTS_H_
#define COEFFICIENTS_H_


float terms[ORDER+1] = {
		0.000896176498665450017334921373191036764,
		0.003136617746484260220440898336846657912,
		0.007449467148734819146871366513096290873,
		0.013918741253322231346545301278183615068,
		0.021809295274729485064613143663336813916,
		0.029577325165467873574476342923844640609,
		0.035309922933913460985078103249179548584,
		0.037425105137752219530700159566549700685,
		0.035309922933913460985078103249179548584,
		0.029577325165467873574476342923844640609,
		0.021809295274729485064613143663336813916,
		0.013918741253322231346545301278183615068,
		0.007449467148734819146871366513096290873,
		0.003136617746484260220440898336846657912,
		0.000896176498665450017334921373191036764
};


float num[ORDER+1] = {
		0.000002292499055621388168423871109502521,
		0.000013754994333728328163510279402714787,
		0.000034387485834320827185039276541189679,
		0.000045849981112427765062543316698651097,
		0.000034387485834320820408775698506786966,
		0.000013754994333728328163510279402714787,
		0.000002292499055621388168423871109502521,
		0,
		0,
		0,
		0,
		0,
		0,
		0
};

float den[ORDER+1] = {
		1,
		 -5.044198728582690804955745988991111516953,
		 10.667126097609660462239844491705298423767,
		-12.097945382098838962292575160972774028778,
		  7.757009272749176176375840441323816776276,
		 -2.664974270881526940968342387350276112556,
		  0.383129731143777740864919678642763756216,
		  0,
		  0,
		  0,
		  0,
		  0,
		  0,
		  0,
		  0
};

#endif
