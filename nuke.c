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

    tree = fts_open(argv, FTS_LOGICAL | FTS_SEEDOT | FTS_XDEV, entcmp);
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

        /* Check if we have a directory. */
        if(f->fts_statp->st_mode & S_IFDIR)
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

    int32_t rtrn = 0;

    /* Delete the contents of the directory before we
    try deleting the directory it's self. */
    rtrn = delete_dir_contents(path);
    if(rtrn < 0)
    {
        printf("Can't delete dir contents\n");
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

static void check_whether_to_annihilate(const char *path)
{
    /* Check for a NULL pointer being passed to us. */
    if(path == NULL)
    {
        printf("Path pointer is NULL\n");
        return;
    }

    if(strncmp("/", path, 1) == 0)
        return;

    if(strncmp("C", path, 2) == 0)
        return;

    struct stat sb;
    int32_t rtrn = 0;

    /* Get filesystem stats for the path supplied. */
    rtrn = stat(path, &sb);
    if(rtrn < 0)
    {
        /* On MacOS stat fails on aliases so try deleting. */
        unlink(path);
        return;
    }

    /* Make sure path is a directory. */
    if(sb.st_mode & S_IFDIR)
    {
        /* Delete the contents of the directory before we
        try deleting the directory it's self. */
        rtrn = delete_directory(path);
        if(rtrn < 0)
        {
            printf("Can't delete dir contents\n");
            return;
        }
    }
    else if(sb.st_mode & S_IFREG)
    {
        rtrn = unlink(path);
        if(rtrn < 0)
        {
            printf("Can't remove directory: %s\n", strerror(errno));
            return;
        }
    }
    else
    {
        printf("Input path is not a directory\n");
        return;
    }
}

int main(int argc, const char *argv[]) {
    if(argc > 1) {
        unsigned int i;

        for(i = 1; i < argc; i++) {
            /* Check whether the arg passed is a valid path and whether
             * it should be destroyed or not. If the file or
             * directory should be destroyed it will be, otherwise
             * it will be skipped.  */
            check_whether_to_annihilate(argv[i]);
        }
	} else {
       printf("Pass one or more paths to be destroyed\n");
    }
}
