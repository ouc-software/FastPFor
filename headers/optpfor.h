/**
 * This is code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 */

/*  Based on code by
 *      Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *      Fabrizio Silvestri <fabrizio.silvestri_at_isti.cnr.it>
 *      Rossano Venturini <rossano.venturini_at_isti.cnr.it>
 *   which was available as APL 2.0.
 */

#ifndef OPTPFOR_H_
#define OPTPFOR_H_

#include "common.h"
#include "util.h"
#include "simple16.h"
#include "newpfor.h"

/**
 * OptPFD
 *
 * Follows:
 *
 * H. Yan, S. Ding, T. Suel, Inverted index compression and query processing with
 * optimized document ordering, in: WWW �09, 2009, pp. 401�410.
 */
template<uint32_t BlockSizeInUnitsOfPackSize, class ExceptionCoder = Simple16<
        false> >
class OPTPFor: public NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder> {
public:
    /*enum {
     PACKSIZE = 32, PFORDELTA_BLOCKSZ = BlockSizeInUnitsOfPackSize
     * PACKSIZE
     };*/
    OPTPFor() /*:
                encodedExceptions(
                        4
                                * NewPFor<BlockSizeInUnitsOfPackSize,
                                        ExceptionCoder>::BlockSize + 1024)*/ {
    }
    uint32_t tryB(uint32_t b, const uint32_t *in, uint32_t len);
    uint32_t findBestB(const uint32_t *in, uint32_t len);
    //vector<uint32_t> encodedExceptions;

    virtual string name() const {
        ostringstream convert;
        convert << "OPTPFor<" << BlockSizeInUnitsOfPackSize << "," << NewPFor<
                BlockSizeInUnitsOfPackSize, ExceptionCoder>::ecoder.name()
                << ">";
        return convert.str();
    }
};

template<uint32_t BlockSizeInUnitsOfPackSize, class ExceptionCoder>
__attribute__ ((pure))
uint32_t OPTPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::tryB(uint32_t b,
        const uint32_t *in, uint32_t len) {

    assert(b <= 32);
    if (b == 32) {
        return len;
    }
    //encodedExceptions.resize(2 * len + 2);
    uint32_t size = div_roundup(len * b, 32);
    uint32_t curExcept = 0;

    for (uint32_t i = 0; i < len; i++) {
        if (in[i] >= (1U << b)) {
            const uint32_t e = in[i] >> b;
            NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::exceptionsPositions[curExcept]
                    = i;
            NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::exceptionsValues[curExcept]
                    = e;
            curExcept++;
        }
    }

    if (curExcept > 0) {

        for (uint32_t i = curExcept - 1; i > 0; i--) {
            const uint32_t cur = NewPFor<BlockSizeInUnitsOfPackSize,
                    ExceptionCoder>::exceptionsPositions[i];
            const uint32_t prev = NewPFor<BlockSizeInUnitsOfPackSize,
                    ExceptionCoder>::exceptionsPositions[i - 1];
            const uint32_t gap = cur - prev;
            NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::exceptionsPositions[i]
                    = gap;
        }

        for (uint32_t i = 0; i < curExcept; i++) {
            const uint32_t excPos =
                    (i > 0) ? NewPFor<BlockSizeInUnitsOfPackSize,
                            ExceptionCoder>::exceptionsPositions[i] - 1
                            : NewPFor<BlockSizeInUnitsOfPackSize,
                                    ExceptionCoder>::exceptionsPositions[i];
            const uint32_t excVal = NewPFor<BlockSizeInUnitsOfPackSize,
                    ExceptionCoder>::exceptionsValues[i] - 1;
            NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::exceptions[i]
                    = excPos;
            NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::exceptions[i
                    + curExcept] = excVal;
        }
        size_t encodedExceptions_sz;
        NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::ecoder.fakeencodeArray(
                &NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::exceptions[0],
                2 * curExcept, /*&encodedExceptions[0], */encodedExceptions_sz);
        size += encodedExceptions_sz;
    }
    return size;

}
template<uint32_t BlockSizeInUnitsOfPackSize, class ExceptionCoder>
__attribute__ ((pure))
uint32_t OPTPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::findBestB(
        const uint32_t *in, uint32_t len) {
    uint32_t
            b =
                    NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::possLogs.back();
    assert(b == 32);
    uint32_t bsize = tryB(b, in, len);

    for (uint32_t i = 0; i
            < NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::possLogs.size()
                    - 1; i++) {
        const uint32_t
                csize =
                        tryB(
                                NewPFor<BlockSizeInUnitsOfPackSize,
                                        ExceptionCoder>::possLogs[i], in, len);

        if (csize <= bsize) {
            b
                    = NewPFor<BlockSizeInUnitsOfPackSize, ExceptionCoder>::possLogs[i];
            bsize = csize;
        }
    }
    return b;
}

#endif /* OPTPFOR_H_ */
