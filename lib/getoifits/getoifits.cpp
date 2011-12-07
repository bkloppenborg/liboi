/*
 * getoifits.c
 * Fabien Baron
 * Oifits routines.
 * This package uses the Oifits Exchange routines by John Young to view,
 * select and extract oi data.
 */

#include "getoifits.h"
#include <cstdlib>
#include <string.h>


#define uv_threshold 5.0e-5
#define infinity 1e99

int compare_uv(oi_uv uv, oi_uv withuv, float thresh)
{
  float sqthresh;
  float pcompu,pcompv;
  float mcompu,mcompv;


  sqthresh = thresh*thresh;

  pcompu = 2.0*(uv.u-withuv.u)*(uv.u-withuv.u)/(uv.u*uv.u+withuv.u*withuv.u);
  pcompv = 2.0*(uv.v-withuv.v)*(uv.v-withuv.v)/(uv.v*uv.v+withuv.v*withuv.v);
  mcompu = 2.0*(uv.u+withuv.u)*(uv.u+withuv.u)/(uv.u*uv.u+withuv.u*withuv.u);
  mcompv = 2.0*(uv.v+withuv.v)*(uv.v+withuv.v)/(uv.v*uv.v+withuv.v*withuv.v);
  
  /* To handle zeros */
  if((uv.u-withuv.u) == 0.0)
    {
      pcompu = 0.0;
    }
  if((uv.v-withuv.v) == 0.0)
    {
      pcompv = 0.0;
    }
  if((uv.u+withuv.u) == 0.0)
    {
      mcompu = 0.0;
    }
  if((uv.v+withuv.v) == 0.0)
    {
      mcompv = 0.0;
    }


  /* If uv is same as withuv then return 1 */
  if((pcompu<sqthresh)&&(pcompv<sqthresh))
    {
      return 1;
    }
  /* If uv is same as withuv but conjugated then return -1 */
  if((mcompu<sqthresh)&&(mcompv<sqthresh))
    {
      return -1;
    }
  
  return 0;
}


int get_oi_fits_data(oi_usersel* usersel, oi_data* data, int* status)
{
	/*
	*  ROUTINE FOR READING OI_FITS DATA.
	*  All uv coords are conjugated into +ve u half plane.
	*/
	/* Declare data structure variables*/
	/*     OI-FITS */
	/* oi_array array; */
	oi_wavelength wave;
	oi_vis2 vis2;
	oi_t3 t3;
	/* Declare other variables */
	int i,k,l;
	int phu;
	int npve, uvexists;
	oi_uv puv1,puv2,puv3;
	fitsfile *fptr;


	/* If error do nothing */
	if(*status) return *status;

	/* Read fits file */
	fits_open_file(&fptr, usersel->file, READONLY, status);
	if(*status) {
	  fits_report_error(stderr, *status);
	  exit(1);
	}

	/* Allocate memory */
	data->pow = (float*) malloc( (usersel->numvis2 + 1) *sizeof(float));
	data->powerr = (float*) malloc( (usersel->numvis2 + 1 ) *sizeof(float));
	data->bisamp = (float*) malloc(usersel->numt3 *sizeof(float));
	data->bisamperr = (float*) malloc(usersel->numt3 *sizeof(float));
	data->bisphs = (float*) malloc(usersel->numt3 *sizeof(float));
	data->bisphserr = (float*) malloc(usersel->numt3 *sizeof(float));
	data->bsref = (oi_bsref*) malloc(usersel->numt3 *sizeof(oi_bsref));
	data->uv = (oi_uv*) malloc((1 + usersel->numvis2+3 *usersel->numt3)*sizeof(oi_uv));
	/* Allocate as much space for UV as possible initially and then reallocate in the end */

	/* Read in visibility */
	if(*status==0)
	{
		/* powerspectrum */
		fits_movabs_hdu(fptr,1,NULL,status);

		/* Add total "zero" flux component for normalization */
		//		data->npow = 1;
		//		data->nuv  = 1;
		//		data->pow[0]=1.0;
		//		data->powerr[0]=usersel->fluxerr;
		//		data->uv[0].u=0.;
		//		data->uv[0].v=0.;
		while(*status==0)
		{
			read_next_oi_vis2(fptr, &vis2, status);
			fits_get_hdu_num(fptr, &phu);
			read_oi_wavelength(fptr, vis2.insname, &wave, status);
			fits_movabs_hdu(fptr,phu,NULL,status);

			if(*status==0)
			{
				if((vis2.record[0].target_id == usersel->target_id))
				{
					for(i=0; i<vis2.numrec; i++)
					{
						for(k=0; k<vis2.nwave; k++)
						{
							if(((wave.eff_wave[k]*billion)> usersel->minband)&&((wave.eff_wave[k]*billion)
										      < usersel->maxband)&&(!(vis2.record[i].flag[k])))
							{
								data->pow[data->npow] = (float)vis2.record[i].vis2data[k];
								data->powerr[data->npow] = (float)vis2.record[i].vis2err[k];

								data->uv[data->nuv].u = (float)(vis2.record[i].ucoord / wave.eff_wave[k]);
								data->uv[data->nuv].v = (float)(vis2.record[i].vcoord / wave.eff_wave[k]);
								//printf("%d %e %e %e\n", k,  wave.eff_wave[k], wave.eff_band[k], wave.eff_band[k] / wave.eff_wave[k]);
								data->uv[data->nuv].wavelength = wave.eff_wave[k];
								data->uv[data->nuv].bandwidth = wave.eff_band[k];
								
								/* flip into +u half plane */
								if(data->uv[data->nuv].u<0.0)
								{
									data->uv[data->nuv].u = -data->uv[data->nuv].u;
									data->uv[data->nuv].v = -data->uv[data->nuv].v;
								}

								data->npow++;
								data->nuv++;
							}
						}
					}
				}
			}
			/* free memory */
			if(*status == 0)
			{
				free_oi_wavelength(&wave);
				free_oi_vis2(&vis2);
			}
		}
		*status = 0;

		/* bispectrum */
		fits_movabs_hdu(fptr,1,NULL,status);
		data->nbis  = 0;
		while(*status==0)
		{
			read_next_oi_t3(fptr, &t3, status);
			fits_get_hdu_num(fptr, &phu);
			read_oi_wavelength(fptr, t3.insname, &wave, status);
			fits_movabs_hdu(fptr,phu,NULL,status);

			if(*status==0)
			{
				if((t3.record[0].target_id == usersel->target_id))
				{
					for(i=0; i<t3.numrec; i++)
					{
						for(k=0; k<t3.nwave; k++)
						{

						  if(((wave.eff_wave[k]*billion)>usersel->minband)
						     &&((wave.eff_wave[k]*billion)<usersel->maxband)
						     &&(!(t3.record[i].flag[k])))
							{
								/* Trick to use closure data without available bis amplitudes */
								if(isnan(t3.record[i].t3amp[k]))
								{
									data->bisamp[data->nbis] = 1.0;
									data->bisamperr[data->nbis] = infinity ;
								}
								else
								{
									data->bisamp[data->nbis] = (float)(t3.record[i].t3amp[k]);
									data->bisamperr[data->nbis] = (float)(t3.record[i].t3amperr[k]);
								}

								data->bisphs[data->nbis] = (float)(t3.record[i].t3phi[k]);
								data->bisphserr[data->nbis] = (float)(t3.record[i].t3phierr[k]);
								// data->bistime[data->nbis] = t3.time;

								/* Read UV coords and check if they exist. If do not exist -> update UV.
								* Set the bsref.
								*/
								puv1.u = (float)( t3.record[ i ].u1coord / wave.eff_wave[ k ] );
								puv1.v = (float)( t3.record[ i ].v1coord / wave.eff_wave[ k ] );
								puv2.u = (float)( t3.record[ i ].u2coord / wave.eff_wave[ k ] );
								puv2.v = (float)( t3.record[ i ].v2coord / wave.eff_wave[ k ] );
								puv3.u = -(puv1.u + puv2.u);
								puv3.v = -(puv1.v + puv2.v);

								/* Check if UV1, UV2, UV3 exist */

								/*uv1*/
								uvexists = 0;
								for(l=0; l<data->nuv; l++)
								{
								  npve = compare_uv(puv1, data->uv[l], uv_threshold);								  
								  if(  (npve != 0 ) && (wave.eff_wave[k] == data->uv[l].wavelength ) )
									{
										data->bsref[data->nbis].ab.uvpnt = l;
										data->bsref[data->nbis].ab.sign = 1;

										/* conjugated ref if u -ve */
										if(npve == -1)
										{
											data->bsref[data->nbis].ab.sign = -1;
										}
										uvexists = 1;
										break; /*so that first match is referenced */
									}
								}
								if(uvexists == 0)
								{
									/* create new uv point */
									data->uv[data->nuv].u = puv1.u;
									data->uv[data->nuv].v = puv1.v;
									data->uv[data->nuv].wavelength = wave.eff_wave[k];
									data->uv[data->nuv].bandwidth = wave.eff_band[k];
									data->bsref[data->nbis].ab.uvpnt = l;
									data->bsref[data->nbis].ab.sign = 1;

									/* conjugate if u -ve */
									if(data->uv[data->nuv].u<0.0)
									{
										data->uv[data->nuv].u = -puv1.u;
										data->uv[data->nuv].v = -puv1.v;
										data->bsref[data->nbis].ab.sign = -1;
									}
									data->nuv++;
								}

								/*uv2*/
								uvexists = 0;
								for(l=0; l<data->nuv; l++)
								{
								  npve = compare_uv(puv2, data->uv[l], uv_threshold);
								  if(  (npve != 0 ) && (wave.eff_wave[k] == data->uv[l].wavelength ) )
									{
										data->bsref[data->nbis].bc.uvpnt = l;
										data->bsref[data->nbis].bc.sign = 1;

										/* conjugated ref if u -ve */
										if(npve == -1)
										{
											data->bsref[data->nbis].bc.sign = -1;
										}
										uvexists = 1;
										break;
									}
								}

								if(uvexists == 0)
								{
									/* create new uv point */
									data->uv[data->nuv].u = puv2.u;
									data->uv[data->nuv].v = puv2.v;
									data->uv[data->nuv].wavelength = wave.eff_wave[k];
									data->uv[data->nuv].bandwidth = wave.eff_band[k];
									data->bsref[data->nbis].bc.uvpnt = l;
									data->bsref[data->nbis].bc.sign = 1;

									/* conjugate if u -ve */
									if(data->uv[data->nuv].u<0.0)
									{
										data->uv[data->nuv].u = -puv2.u;
										data->uv[data->nuv].v = -puv2.v;
										data->bsref[data->nbis].bc.sign = -1;
									}
									data->nuv++;
								}

								/*uv3 = (-uv2-uv2)*/
								uvexists = 0;
								for(l=0; l<data->nuv; l++)
								{
								  npve = compare_uv(puv3, data->uv[l], uv_threshold);
								  if(  (npve != 0 ) && (wave.eff_wave[k] == data->uv[l].wavelength ) )
									{
										data->bsref[data->nbis].ca.uvpnt = l;
										data->bsref[data->nbis].ca.sign = 1;

										/* conjugated ref if u -ve */
										if(npve == -1)
										{
											data->bsref[data->nbis].ca.sign = -1;
										}
										uvexists = 1;
										break;
									}
								}

								if(uvexists == 0)
								{
									/* create new uv point */
									data->uv[data->nuv].u = puv3.u;
									data->uv[data->nuv].v = puv3.v;
									data->uv[data->nuv].wavelength = wave.eff_wave[k];
									data->uv[data->nuv].bandwidth = wave.eff_band[k];
									data->bsref[data->nbis].ca.uvpnt = l;
									data->bsref[data->nbis].ca.sign = 1;

									/* conjugate if u -ve */
									if(data->uv[data->nuv].u<0.0)
									{
										data->uv[data->nuv].u = -puv3.u;
										data->uv[data->nuv].v = -puv3.v;
										data->bsref[data->nbis].ca.sign = -1;
									}
									data->nuv++;
								}

								data->nbis++;
							}
						}
					}
				}
			}
			/* free memory */
			if(*status == 0)
			{
				free_oi_wavelength(&wave);
				free_oi_t3(&t3);
			}
		}
		*status = 0;
	}

	/* ERROR HANDLING */
	return *status;
}

int get_oi_fits_selection(oi_usersel* usersel, int* status)
{
	/*
	* ROUTINE FOR READING OI_FITS INFO.
	*/
	/* Declare data structure variables*/
	/*     OI-FITS */
	/* oi_array array; */
	oi_target targets;
	oi_wavelength wave;
	oi_vis2 vis2;
	oi_t3 t3;
	/* Data description */
	int nv2tab=0, nt3tab=0;
	int phu;
	/* Declare other variables */

	char comment[FLEN_COMMENT];
	char extname[FLEN_VALUE];
	char zerostring[FLEN_VALUE];
	char commstring[100];
	int hdutype;
	int i,k;
	int nhu=0;
	int tmpi;

	fitsfile *fptr;

	/* If error do nothing */
	if(*status) return *status;

	/* Initialise */
	usersel->numins = 0;
	for(k=0; k<FLEN_VALUE; k++)zerostring[k] = ' ';

	/* Read fits file */
	fits_open_file(&fptr, usersel->file, READONLY, status);
	if(*status) {
		fits_report_error(stderr, *status);
		exit(1);
	}

	/* GET NO OF HEADER UNITS */
	fits_get_num_hdus(fptr,&nhu,status);
	/* PRINT HEADER UNIT LABELS */
	if(*status == 0)
	{
		printf("Reading unit labels:\t");
		for(i = 1; i<(nhu+1); i++)
		{
			fits_movabs_hdu(fptr,i,&hdutype,status);
			if (hdutype == BINARY_TBL) {
				fits_read_key(fptr, TSTRING, "EXTNAME", extname, comment, status);
				printf("%s ",extname);
			}
		}

	}
	/* GET TARGETS AND SELECT */
	if(*status == 0)
	{
		fits_movabs_hdu(fptr,1,NULL,status);
		read_oi_target(fptr,&targets,status);

		for(i=0; i<(targets.ntarget); i++)
		{
			printf("\nTarget id/name:\t\t%d/%s\n",targets.targ[i].target_id,targets.targ[i].target);
		}

		if(targets.ntarget>1)
		{
			AGAIN1:
			printf("\nSELECT AN ID: ");
			scanf("%d",&usersel->target_id);
			tmpi = 0;
			for(i=0; i<targets.ntarget; i++)
			{
				if(targets.targ[i].target_id == usersel->target_id)
				{
					tmpi=1;
					strcpy(usersel->target, targets.targ[i].target);
				}
			}
			if(tmpi==0)goto AGAIN1;
		}
		else
		{
			printf("Auto selecting the only target \"%s\".\n",targets.targ[0].target);
			usersel->target_id = targets.targ[0].target_id;
			strcpy(usersel->target,zerostring);
			strcpy(usersel->target,targets.targ[0].target);
		}
		printf("\n");
		/* free memory */
		if(*status == 0)
		{
			free_oi_target(&targets);
		}
	}

	/* PRINT AVAILABLE DATA ON SELECTED TARGET */
	if(*status == 0)
	{
		/* V2 TABLES */
		printf("POWERSPECTRUM TABLES\n");
		printf("#\tDate\t\tArray\t\t\tInstrument\t\tNrec/Nwav\n");
		fits_movabs_hdu(fptr,1,NULL,status);
		while(*status == 0)
		{
			read_next_oi_vis2(fptr, &vis2, status);
			fits_get_hdu_num(fptr, &phu);
			if(*status==0)
			{
				if((vis2.record[0].target_id == usersel->target_id))
				{
					nv2tab++;
					printf("%-6.3d\t%-14.11s\t%-20.20s\t%-20.20s\t%ld/%d\n",nv2tab,vis2.date_obs,vis2.arrname,
					vis2.insname,vis2.numrec,vis2.nwave);
					/* Check if need to register new array for this one */
					tmpi = 0;
					for(i=0; i<usersel->numins; i++)
					{
						tmpi += ( (int)strcmp(vis2.insname,usersel->insname[i]) == 0) ;
					}

					if((tmpi != 0)||(usersel->numins==0))
					{
						strcpy(usersel->insname[usersel->numins], zerostring);
						strcpy(usersel->insname[usersel->numins], vis2.insname);
						usersel->numins++;
					}
				}
			}

			/* free memory */
			if(*status == 0)
			{
				free_oi_vis2(&vis2);
			}
		}
		phu = 1;
		*status = 0;
		if(nv2tab<0)printf("\nNo powerspectrum data available for \"%s\"\n",usersel->target);
	}


	/* T3 TABLES */
	if(*status == 0)
	{
		printf("\n");
		printf("BISPECTRUM TABLES\n");
		printf("#\tDate\t\tArray\t\t\tInstrument\t\tNrec/Nwav\n");
		fits_movabs_hdu(fptr,1,NULL,status);

		while(*status == 0)
		{
			read_next_oi_t3(fptr, &t3, status);
			if(*status==0)
			{
				if((t3.record[0].target_id == usersel->target_id))
				{
					nt3tab++;
					printf("%-6.3d\t%-14.11s\t%-20.20s\t%-20.20s\t%ld/%d\n",nt3tab,
					t3.date_obs,t3.arrname,t3.insname,t3.numrec,t3.nwave);
					/* Check if need to register new array for this one */
					tmpi = 0;
					for(i=0; i<usersel->numins; i++)
					{
					  tmpi += ( (int)strcmp(t3.insname,usersel->insname[i]) == 0) ;
					}
					if((tmpi != 0)||(usersel->numins==0))
					{
						strcpy(usersel->insname[usersel->numins],zerostring);
						strcpy(usersel->insname[usersel->numins], t3.insname);
						usersel->numins++;
					}
				}
			}
			if(*status == 0)
			{
				free_oi_t3(&t3);
			}
		}
		phu = 1;
		*status = 0;
		if(nv2tab<0)printf("\nNo bispectrum data available for \"%s\"\n",usersel->target);
	}


	/* CHANNELS IN INSTRUMENTS */
	if(*status == 0)
	  {
	    printf("\nINSTRUMENT SPECTRAL CHANNELS\n");
	    printf("#\tInstrument\t\tChannel_id\tBand/Bandwidth (nm)\n");
	    for(i=0; i<usersel->numins; i++)
	      {
		fits_movabs_hdu(fptr,1,NULL,status);
		/* Read wave table */
		read_oi_wavelength(fptr, usersel->insname[i], &wave, status);
		/* Display wave table */
		for(k=0; k<wave.nwave; k++)
		  {
		    if(k==0)printf("%-6d\t%-25.20s",i,usersel->insname[i]);
		    else printf("%-6.6s\t%-25.20s",zerostring,zerostring);
		    
		    printf("%-3.3d_%-14.3d\t%.0f/%.0f\n",i,k,(wave.eff_wave[k]*billion),(wave.eff_band[k]*billion));
		    
		    /*
		      if (strlen(usersel->insname[i]) > 0)
		      {
		      read_oi_array(fptr, usersel->insname[i], &array, status);
		      printf("Note: array %s has %d telescopes\n", vis2.arrname, array.nelement);
		      usersel->ntelescopes = array.nelement;
		      }
		    */
		  }
		/* free memory */
		if(*status == 0)
		  {
		    free_oi_wavelength(&wave);
		  }
	      }

	  AGAIN2: //waveband selection
	    if( ( usersel->minband < 0.) || (usersel->maxband <= usersel->minband) ) // waveband not set (well) externally
	      {
		if(wave.nwave > 1)
		  {
		    printf("Select a wavelength range (default value = 1 50000) :");
		    //fgets(commstring,100,stdin);
		    //tmpi = sscanf(commstring,"%f %f", &usersel->minband, &usersel->maxband);
		    // TODO: Undo the automatic selection here:
		    tmpi = sscanf("1 50000","%f %f", &usersel->minband, &usersel->maxband);
		  }
		else
		  {
		    tmpi = -1;
		    printf("Only one spectral channel. ");
		  }
		
		if(tmpi == 2)
		  {
		    if((usersel->maxband <= usersel->minband)||(usersel->minband < 0.0))
		      {
			printf("Invalid band selection!\n");
			goto AGAIN2;
		      }
		  }
		else if(tmpi == -1)
		  {
		    printf("Automatic selection of the full channel\n");
		    usersel->minband = 1. ; /* (wave.eff_wave[0]-wave.eff_band[0]/2.)*billion ; */
		    usersel->maxband = 50000. ; /*(wave.eff_wave[0]+wave.eff_band[0]/2.)*billion ; */
		  }
		else
		  {
		    printf("Invalid band selection!\n");
		    goto AGAIN2;
		  }
	      }
	    
	  }
	
	/* Count number of vis2 and t3 available in this range for this target */
	if(*status==0)
	  {
	    /* powerspectrum */
	    usersel->numvis2 = 0;
	    fits_movabs_hdu(fptr,1,NULL,status);
	    while(*status==0)
	      {
		read_next_oi_vis2(fptr, &vis2, status);
		fits_get_hdu_num(fptr, &phu);
		read_oi_wavelength(fptr, vis2.insname, &wave, status);
		fits_movabs_hdu(fptr,phu,NULL,status);
		
		if(*status==0)
		  {
		    if(vis2.record[0].target_id == usersel->target_id)
		      {
			for(i=0; i<vis2.numrec; i++)
			  {
			    for(k=0; k<vis2.nwave; k++)
			      {
				if(((wave.eff_wave[k]*billion)>usersel->minband)&&((wave.eff_wave[k]*billion)<usersel->maxband)&&(!(vis2.record[i].flag[k])))
				  {
				    usersel->numvis2++;
				  }
			      }
			  }
		      }
		  }
		/* free memory */
		if(*status == 0)
		  {
		    free_oi_vis2(&vis2);
		    free_oi_wavelength(&wave);
		  }
	      }
	    *status = 0;
	    
	    /* bispectrum */
	    usersel->numt3 = 0;
	    fits_movabs_hdu(fptr,1,NULL,status);
	    while(*status==0)
	      {
		read_next_oi_t3(fptr, &t3, status);
		fits_get_hdu_num(fptr, &phu);
		read_oi_wavelength(fptr, t3.insname, &wave, status);
		fits_movabs_hdu(fptr,phu,NULL,status);
		
		if(*status==0)
			{
			  if(t3.record[0].target_id == usersel->target_id)
			    {
			      for(i=0; i<t3.numrec; i++)
				{
				  for(k=0; k<t3.nwave; k++)
				    {
				      if( ((wave.eff_wave[k]*billion)>usersel->minband)
					  &&((wave.eff_wave[k]*billion)<usersel->maxband)
					  && (!(t3.record[i].flag[k]))  )
					{
					  usersel->numt3++;
							}
				    }
				}
			    }
			}
		/* free memory */
		if(*status == 0)
		  {
				free_oi_t3(&t3);
				free_oi_wavelength(&wave);
		  }
	      }
		*status = 0;
		if((usersel->numt3==0)&&(usersel->numvis2==0))
		{
		  printf("Error: no data available within the selected waveband limits\n");
		  usersel->minband= -1.;
		  usersel->maxband= -1.;
		  //getchar();
		  goto AGAIN2;
		}
		printf("Found %ld powerspectrum and %ld bispectrum points between %.0f and %.0f nm.\n\n",
						usersel->numvis2,usersel->numt3, usersel->minband, usersel->maxband);
		
	  }
	
	/* CLOSE FILE */
	fits_close_file(fptr, status);
	
	
	
	/* ERROR HANDLING */
	return *status;
}

void free_oi_target(oi_target *targets)
{
    free(targets->targ);
}

void free_oi_wavelength(oi_wavelength *wave)
{
  free(wave->eff_wave);
  free(wave->eff_band);
}

void free_oi_vis2(oi_vis2 *vis2)
{
  int i;

  for(i=0; i<vis2->numrec; i++)
    {
      free(vis2->record[i].vis2data);
      free(vis2->record[i].vis2err);
      free(vis2->record[i].flag);
    }
  free(vis2->record);
}

void free_oi_t3(oi_t3 *t3)
{
  int i;

  for(i=0; i<t3->numrec; i++)
    {
      free(t3->record[i].t3amp);
      free(t3->record[i].t3amperr);
      free(t3->record[i].t3phi);
      free(t3->record[i].t3phierr);
      free(t3->record[i].flag);
    }
  free(t3->record);
}

void free_oi_data(oi_data *data)
{
  free(data->pow);
  free(data->powerr);
  free(data->bisamp);
  free(data->bisamperr);
  free(data->bisphs);
  free(data->bisphserr);
  free(data->uv);
  free(data->bsref);
}
