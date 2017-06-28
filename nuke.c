#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fts.h>

static int32_t entcmp(const FTSENT *const *a, const FTSENT *const *b)
{
    return (strcmp((*a)->fts_name, (*b)->fts_name));
}

static int32_t delete_dir_contents(const char *dir)
{
    FTS *tree;
    FTSENT *f;
    int32_t rtrn = 0;
    char *argv[] = { dir, NULL };

    tree = fts_open(argv, FTS_LOGICAL | FTS_NOSTAT, entcmp);
    if(tree == NULL)
    {
        printf( "Can't walk directory\n");
        return (-1);
    }

    while((f = fts_read(tree)))
    {
        switch(f->fts_info)
        {
            case FTS_DNR:   /* Cannot read directory */
            case FTS_ERR:   /* Miscellaneous error */
            case FTS_NS:    /* stat() error */
                continue;
        }

        if(strcmp(f->fts_path, dir) == 0)
            continue;

        struct stat sb;

        rtrn = stat(f->fts_path, &sb);
        if(rtrn < 0)
        {
            printf("Can't get path stats: %s\n", strerror(errno));
            return (-1);
        }

        /* Check if we have a directory. */
        if(sb.st_mode & S_IFDIR)
        {
            rtrn = rmdir(f->fts_path);
            if(rtrn < 0)
                continue;
        }
        else
        {
            rtrn = unlink(f->fts_path);
            if(rtrn < 0)
            {
                printf("Can't remove file\n");
                return (-1);
            }
        }
    }

    rtrn = fts_close(tree);
    if(rtrn < 0)
    {
        printf("fts: %s\n", strerror(errno));
        return (-1);
    }

    return (0);
}

static int32_t delete_directory(char *path)
{
    /* Check for a NULL pointer being passed to us. */
    if(path == NULL)
    {
        printf("Path pointer is NULL\n");
        return (-1);
    }

    struct stat sb;
    int32_t rtrn = 0;

    /* Get filesystem stats for the path supplied. */
    rtrn = stat(path, &sb);
    if(rtrn < 0)
    {
        printf("Can't get stats: %s\n", strerror(errno));
        return(-1);
    }

    /* Make sure path is a directory. */
    if(sb.st_mode & S_IFDIR)
    {
        /* Delete the contents of the directory before we
        try deleting the directory it's self. */
        rtrn = delete_dir_contents(path);
        if(rtrn < 0)
        {
            printf("Can't delete dir contents\n");
            return (-1);
        }
    }
    else
    {
        printf("Input path is not a directory\n");
        return (-1);
    }

    rtrn = rmdir(path);
    if(rtrn < 0)
    {
        printf("Can't remove directory: %s\n", strerror(errno));
        return (-1);
    }

    return (0);
}

int main(int argc, const char *argv[]) {
    if(argc > 1) {
        printf("Nuking from orbit: %s\n", argv[1]);
		delete_directory(argv[1]);
	}
}