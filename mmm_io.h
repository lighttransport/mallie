#ifndef __MALLIE_MMM_IO_H__
#define __MALLIE_MMM_IO_H__

//
// MMM = Mallie MonochroMe image format.
//

namespace mallie {

bool SaveMMM(const char *filename, double *data, int width, int height);

bool LoadMMM(double **data, int &width, int &height, const char *filename);
};

#endif // __MALLIE_MMM_IO_H__
