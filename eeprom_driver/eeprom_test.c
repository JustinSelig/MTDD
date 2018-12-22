/* eeprom_test.c
 *
 * Justin S. Selig
 * Application Tier
 */

#include "eeprom.h"

//Global device mutex for any process interfacing with eeprom
pthread_mutex_t eeprom_lock;

//User-defined callback function for driver errors
void generic_fault_handler(char *err)
{
    printf("FAULT: %s\n", err);
    pthread_mutex_destroy(&eeprom_lock);
    exit(0); //exit program
}

//Constructs device, tests simple write than read
int test_1()
{
    //Device initializations
    eeprom_dev_properties_t props = {
        .base_address = 0,
        .device_size_bits = 65536,
        .device_size_words = 8192,
        .word_size_bits = 8,
        .page_size_bytes = 32,
    };
    eeprom_dev_t *dev = malloc(sizeof(eeprom_dev_t));
    if (dev == NULL)
    {
        printf("failed device allocation\n");
    }
    dev->mutex = &eeprom_lock;
    dev->properties = props;
    dev->fault_handler = generic_fault_handler;

    const uint32_t offset  = 0;
    char           wbuf[]  = {0x44, 0x44, 0x44, 0x44, 0x44}; //ascii 'D'
    int            size    = 5;
    int            res     = 0;
    char           rbuf[1024];
    int            i;
    res = eeprom_write(dev, offset, size, wbuf);
    if (res < 0)
    {
        printf("test 1 failed to write to device\n");
        return -1;
    }
    res = eeprom_read(dev, offset, size, rbuf);
    if (res < 0)
    {
        printf("test 1 failed to read from device\n");
        return -1;
    }
    for (i=0; i<size; i++)
    {
        if (wbuf[i] != rbuf[i])
        {
            return -1;
        }
    }

    free(dev);
    return 1; //success
}

//Tests single page boundary write, read
int test_2()
{
    //Device initializations
    eeprom_dev_properties_t props = {
        .base_address = 0,
        .device_size_bits = 65536,
        .device_size_words = 8192,
        .word_size_bits = 8,
        .page_size_bytes = 32,
    };
    eeprom_dev_t *dev = malloc(sizeof(eeprom_dev_t));
    if (dev == NULL)
    {
        printf("failed device allocation\n");
    }
    dev->mutex = &eeprom_lock;
    dev->properties = props;
    dev->fault_handler = generic_fault_handler;

    const uint32_t offset  = 30;
    char           wbuf[]  = {0x44, 0x44, 0x44, 0x44, 0x44}; //ascii 'D'
    int            size    = 5;
    int            res     = 0;
    char           rbuf[1024];
    int            i;
    res = eeprom_write(dev, offset, size, wbuf);
    if (res < 0)
    {
        printf("test 2 failed to write to device\n");
        free(dev);
        return -1;
    }
    res = eeprom_read(dev, offset, size, rbuf);
    if (res < 0)
    {
        printf("test 2 failed to read from device\n");
        free(dev);
        return -1;
    }
    for (i=0; i<size; i++)
    {
        if (wbuf[i] != rbuf[i])
        {
            free(dev);
            return -1;
        }
    }

    free(dev);
    return 1; //success
}

//Tests multi-page boundary write, read
int test_3()
{
    //Device initializations
    eeprom_dev_properties_t props = {
        .base_address = 0,
        .device_size_bits = 65536,
        .device_size_words = 8192,
        .word_size_bits = 8,
        .page_size_bytes = 32,
    };
    eeprom_dev_t *dev = malloc(sizeof(eeprom_dev_t));
    if (dev == NULL)
    {
        printf("failed device allocation\n");
    }
    dev->mutex = &eeprom_lock;
    dev->properties = props;
    dev->fault_handler = generic_fault_handler;

    char wbuf[100]  = {
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
         0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44};
    const uint32_t offset  = 30;
    int            size    = 100;
    int            res     = 0;
    char           rbuf[1024];
    int            i;
    //write to 100 locatons across multiple page boundaries
    res = eeprom_write(dev, offset, size, wbuf);
    if (res < 0)
    {
        printf("test 3 failed to write to device\n");
        free(dev);
        return -1;
    }

    res = eeprom_read(dev, offset, size, rbuf);
    if (res < 0)
    {
        printf("test 3 failed to read from device\n");
        free(dev);
        return -1;
    }
    for (i=0; i<size; i++)
    {
        if (wbuf[i] != rbuf[i])
        {
            free(dev);
            return -1;
        }
    }

    free(dev);
    return 1; //success
}

//Tests writing to max boundary
int test_4()
{
    //Device initializations
    eeprom_dev_properties_t props = {
        .base_address = 0,
        .device_size_bits = 65536,
        .device_size_words = 8192,
        .word_size_bits = 8,
        .page_size_bytes = 32,
    };
    eeprom_dev_t *dev = malloc(sizeof(eeprom_dev_t));
    if (dev == NULL)
    {
        printf("failed device allocation\n");
    }
    dev->mutex = &eeprom_lock;
    dev->properties = props;
    dev->fault_handler = generic_fault_handler;

    const uint32_t offset  = 8000;
    char           wbuf[]  = {0x44}; //ascii 'D'
    int            size    = 1;
    int            res     = 0;
    int            i;

    //write up until memory bound (8191)
    for (i=offset; i < offset+192; i++)
    {
        res = eeprom_write(dev, i, size, wbuf);
        if (res < 0)
        {
            printf("test 4 failed to write to device\n");
            free(dev);
            return -1;
        }
    }

    // Uncomment to test write just beyond memory bound (8192)
    // for (i=offset; i < offset+193; i++)
    // {
    //     res = eeprom_write(dev, i, size, wbuf);
    //     if (res < 0)
    //     {
    //         printf("Test 3: Failed to write to device\n");
    //         free(dev);
    //         return -1;
    //     }
    // }

    return 1; //success
}

//Tests bad user input
int test_5()
{
    //Device intentionally not allocated
    eeprom_dev_t *dev = NULL;

    const uint32_t offset  = 0;
    char           wbuf[]  = {0x44, 0x44, 0x44, 0x44, 0x44}; //ascii 'D'
    int            size    = 5;
    int            res     = 0;

    res = eeprom_write(dev, offset, size, wbuf);
    if (res < 0)
    {
        printf("test 5 failed to write to device\n");
        free(dev);
        return -1;
    }

    free(dev);
    return 1;
}

//writer1 process
void * p1_write_to_eeprom(void *arg)
{
    eeprom_dev_properties_t props = {
        .base_address = 0,
        .device_size_bits = 65536,
        .device_size_words = 8192,
        .word_size_bits = 8,
        .page_size_bytes = 32,
    };
    eeprom_dev_t *dev = malloc(sizeof(eeprom_dev_t));
    if (dev == NULL)
    {
        printf("failed device allocation\n");
    }
    dev->mutex = &eeprom_lock;
    dev->properties = props;
    dev->fault_handler = generic_fault_handler;
    dev->id = 1; //optional

    char buf[] = {0x55, 0x55, 0x55, 0x55, 0x55}; //ascii 'U'
    int res = eeprom_write(dev, 30, sizeof(buf), buf);
    if (res < 0)
    {
        printf("p1 failed to write to device\n");
    }
    else
    {
        printf("p1 wrote successfully\n");
    }

    free(dev);
    return 0;
}

//writer2 process
void * p2_write_to_eeprom(void *arg)
{
    eeprom_dev_properties_t props = {
        .base_address = 0,
        .device_size_bits = 65536,
        .device_size_words = 8192,
        .word_size_bits = 8,
        .page_size_bytes = 32,
    };
    eeprom_dev_t *dev = malloc(sizeof(eeprom_dev_t));
    if (dev == NULL)
    {
        printf("failed device allocation\n");
    }
    dev->mutex         = &eeprom_lock;
    dev->properties    = props;
    dev->fault_handler = generic_fault_handler;
    dev->id            = 2; //optional

    char buf[] = {0x44, 0x44, 0x44, 0x44, 0x44}; //ascii 'D'
    int res = eeprom_write(dev, 30, sizeof(buf), buf);
    if (res < 0)
    {
        printf("p2 failed to write to device\n");
    }
    else
    {
        printf("p2 wrote successfully\n");
    }

    free(dev);
    return 0;
}

//reader1 process
void * p3_read_from_eeprom(void *arg)
{
    eeprom_dev_properties_t props = {
        .base_address = 0,
        .device_size_bits = 65536,
        .device_size_words = 8192,
        .word_size_bits = 8,
        .page_size_bytes = 32,
    };
    eeprom_dev_t *dev = malloc(sizeof(eeprom_dev_t));
    if (dev == NULL)
    {
        printf("failed device allocation\n");
    }
    dev->mutex         = &eeprom_lock;
    dev->properties    = props;
    dev->fault_handler = generic_fault_handler;
    dev->id            = 3; //optional

    const uint32_t offset  = 10;
    const uint32_t size    = 50;
    int            res     = 0;
    char           buf[1024];

    res = eeprom_read(dev, offset, size, buf);
    if (res < 0)
    {
        printf("p3 failed to read from device\n");
    }
    else
    {
        printf("p3 read successfully\n");
    }

    free(dev);
    return 0;
}

//reader2 process
void * p4_read_from_eeprom(void *arg)
{
    eeprom_dev_properties_t props = {
        .base_address = 0,
        .device_size_bits = 65536,
        .device_size_words = 8192,
        .word_size_bits = 8,
        .page_size_bytes = 32,
    };
    eeprom_dev_t *dev = malloc(sizeof(eeprom_dev_t));
    if (dev == NULL)
    {
        printf("failed device allocation\n");
    }
    dev->mutex         = &eeprom_lock;
    dev->properties    = props;
    dev->fault_handler = generic_fault_handler;
    dev->id            = 4; //optional

    const uint32_t offset  = 10;
    const uint32_t size    = 50;
    int            res     = 0;
    char           buf[1024];

    res = eeprom_read(dev, offset, size, buf);
    if (res < 0)
    {
        printf("p4 failed to read from device\n");
    }
    else
    {
        printf("p4 read successfully\n");
    }

    free(dev);
    return 0;
}

int main()
{
    int res = 0;

    //Test simple write than read
    printf("TEST 1: Write and Read Within Page\n");
    res = test_1();
    if (res == 1)
    {
        printf("test 1 succeeded\n");
    }
    else
    {
        printf("test 1 failed\n");
    }

    //Test write then read across page boundaries
    printf("TEST 2: Write and Read Across Single Page Boundary\n");
    res = 0;
    res = test_2();
    if (res == 1)
    {
        printf("test 2 succeeded\n");
    }
    else
    {
        printf("test 2 failed\n");
    }

    //Test write then read across multiple page boundaries
    printf("TEST 3: Write and Read Across Multiple Page Boundaries\n");
    res = 0;
    res = test_3();
    if (res == 1)
    {
        printf("test 3 succeeded\n");
    }
    else
    {
        printf("test 3 failed\n");
    }

    //Test write and read outside of eeprom boundaries
    printf("TEST 4: Write and Read at EEPROM Boundary\n");
    res = 0;
    res = test_4();
    if (res == 1)
    {
        printf("test 4 succeeded\n");
    }
    else
    {
        printf("test 4 failed\n");
    }

    //Test bad user input
    printf("TEST 5: Bad User Input, should fail\n");
    res = 0;
    res = test_5();
    if (res == 1)
    {
        printf("test 5 succeeded\n");
    }
    else
    {
        printf("test 5 failed\n");
    }

    printf("TEST 6: Multiple Writers and Multiple Readers\n");
    //initialize device mutex
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&eeprom_lock, &mattr);
    if (pthread_mutex_init(&eeprom_lock, NULL) != 0)
    {
        printf("mutex init failed\n");
        return -1;
    }

    pthread_t writer1, writer2, reader1, reader2; //thread ids
    //two writer threads
    pthread_create(&writer1, NULL, &p1_write_to_eeprom, NULL); //thread1
    pthread_create(&writer2, NULL, &p2_write_to_eeprom, NULL); //thread2
    //two reader threads
    pthread_create(&reader1, NULL, &p3_read_from_eeprom, NULL); //thread3
    pthread_create(&reader2, NULL, &p4_read_from_eeprom, NULL); //thread4
    //start
    pthread_join(writer1, NULL);
    pthread_join(writer2, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);

    return 0;
}
