#ifndef PGF_CLF_TO_RME_H
#define PGF_CLF_TO_RME_H

void Convert_PGF_CLF_to_RME(const wxString &pgf_fname,
			    const wxString &clf_fname,
			    const wxString &output_path);

void Convert_PGF_CLF_to_RME_with_PS(const wxString &pgf_fname,
				    const wxString &clf_fname,
				    const wxString &ps_fname,
				     const wxString &output_path);

void Convert_PGF_CLF_to_RME_with_MPS(const wxString &pgf_fname,
				     const wxString &clf_fname,
				     const wxString &mps_fname,
				     const wxString &output_path);

#endif
