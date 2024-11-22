// rcv_funcs.c: Required functions for Ranked Choice Voting

#include "rcv.h"
#include <stdlib.h>
#include <stdio.h>
////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES

int LOG_LEVEL = 0;
// Global variable controlling how much info should be printed; it is
// assigned values like LOG_SHOWVOTES (defined in rcv.h as 3) to
// trigger additional output to be printed during certain
// functions. This output is useful to monitor and audit how election
// results are calculated.

////////////////////////////////////////////////////////////////////////////////
// PROBLEM 1 Functions

void vote_print(vote_t *vote){
 if (vote == NULL) {
        return;
    }

    printf("#%04d:", vote->id);

    for (int i = 0; i < MAX_CANDIDATES; i++) {
        if (vote->candidate_order[i] == NO_CANDIDATE) {
            break;
        }

        if (i == vote->pos) {
            printf("<%d> ", vote->candidate_order[i]);
        } else {
            printf(" %d ", vote->candidate_order[i]);
        }
    }
}
// PROBLEM 1: Print a textual representation of the vote. A vote which
// is defined as follows
//
// vote_t vote = {.id= 17, .pos=1, .next=...,
//                .candidate_order={3, 0, 2, 1, NO_CANDIDATE}};
//
// would be printed  like this:
//
// #0017: 3 <0> 2  1
//
// The first token printed is a # character followed by the vote->id
// fields printed in a space of 4 digits with leading 0s using the
// built-in capabilities of printf() ending with a colon (:).  The
// remaining tokens are candidate indexs in order of preference, "3 0
// 2 1" in this case.  The candidate index at vote->pos is printed
// with angle brackets around it as in "<0>" while other indexes are
// printed with spaces aroudn them as in " 3 ". If `candidate_order[]`
// array has fewer than the MAX_CANDIDATE in it, the slot after the
// last preferred candidate will have `NO_CANDIDATE` in it and
// printing should terminate there. The `next` field is not printed
// and not used during printing.
//
// NOTE: For maximum flexibility, NO NEWLINE is printed at the end of
// the vote which allows several votes to printed on the same line if
// needed.

int vote_next_candidate(vote_t *vote, char *candidate_status){
   if (vote == NULL || candidate_status == NULL) {
        return NO_CANDIDATE;
    }

    // Move until a valid candidate is found
    while (vote->pos < MAX_CANDIDATES) {
        int candidate = vote->candidate_order[vote->pos];

        if (candidate == NO_CANDIDATE || candidate >= MAX_CANDIDATES) {
            return NO_CANDIDATE;
        }

        if (candidate_status[candidate] == CAND_ACTIVE) {
            return candidate;
        }

        vote->pos++;  // Move candidate in order
    }

    return NO_CANDIDATE;
}
// PROBLEM 1: Advance the vote to the next active candidate. This
// function usually changes `vote->pos` to indicate a new candidate is
// selected. If `candidate_order[pos]` is not NO_CANDIDATE and is less
// than MAX_CANDIDATES , increment `pos` and check if the
// `candidate_order[pos]` is ACTIVE. The status of each candidate is
// available in the `candidate_status[]` array where each index is one
// of CAND_ACTIVE, CAND_MINVOTES, CAND_DROPPED. If
// vote->pos exceeds MAX_CANDIDATES or a NO_CANDIDATE value is
// encountered in `candidate_order[]`, return NO_CANDIDATE. Otherwise
// return the index of the selected candidate for the vote.
//
// EXAMPLES:
//                                       D  D  A  D
// vote_t v = {.pos=1, .candidate_order={2, 0, 3, 1, NO_CANDIDATE}};
// int cand_status[4] = {DROPPED, DROPPED, DROPPED, ACTIVE};
// 1ST CALL
// int next_cand = vote_next_candidate(&vote, cand_status);
// - next_cand is 3
// - v is {.pos=2, .candidate_order={2, 0, 3, 1, NO_CANDIDATE}}
// - pos has advanced from 1 to 2 which is the next ACTIVE candidate
// 2ND CALL
// next_cand = vote_next_candidate(&vote, cand_status);
// - next_cand is NO_CANDIDATE
// - v is {.pos=4, .candidate_order={2, 0, 3, 1, NO_CANDIDATE}}
// - pos has incremented from 3 to 4
// 3RD CALL
// next_cand = vote_next_candidate(&vote, cand_status);
// - next_cand is NO_CANDIDATE
// - v is {.pos=4, .candidate_order={2, 0, 3, 1, NO_CANDIDATE}}
// - pos has not changed as it referred to NO_CANDIDATE already

void tally_print_table(tally_t *tally){
if (tally == NULL) {
        return;
    }

    int total_votes = 0;
    for (int i = 0; i < tally->candidate_count; i++) {
        total_votes += tally->candidate_vote_counts[i];
    }

    printf("NUM COUNT %%PERC S NAME\n");

    for (int i = 0; i < tally->candidate_count; i++) {
        char status_char;
        switch (tally->candidate_status[i]) {
            case CAND_ACTIVE:
                status_char = 'A';
                break;
            case CAND_MINVOTES:
                status_char = 'M';
                break;
            case CAND_DROPPED:
                status_char = 'D';
                break;
            default:
                status_char = '?';
        }

        if (tally->candidate_status[i] == CAND_DROPPED) {
            printf("%3d     -     - %c %-10s\n", i, status_char, tally->candidate_names[i]);
        } else {
            float percentage = (total_votes > 0) ? (100.0 * tally->candidate_vote_counts[i] / total_votes) : 0.0;
            printf("%3d %5d %5.1f %c %-10s\n", i, tally->candidate_vote_counts[i], percentage, status_char, tally->candidate_names[i]);
        }
    }

    if (tally->invalid_vote_count > 0) {
        printf("Invalid vote count: %d\n", tally->invalid_vote_count);
    }
}
// PROBLEM 1: Print a table showing the vote breakdown for the
// tally. The table appears like the following.
//
// NUM COUNT %PERC S NAME
//   0     4  57.1 A Francis
//   1     1  14.3 M Claire
//   2     -     - D Heather
//   3     2  28.6 A Viktor
//
// This table would be printed for a tally_t with the following data
//
// tally_t t = {
//   .candidate_count = 4;
//   .candidate_names = {"Francis",   "Claire",      "Heather",    "Viktor"},
//   .candidate_status= {CAND_ACTIVE, CAND_MINVOTES, CAND_DROPPED, CAND_ACTIVE},
//   .candidate_vote_counts = {4,     1,             0,            2}
// }
//
// Each candidate is printed along with their "number", count of their
// votes, percentage of that count compared to the total votes for all
// candidates, their candidate state, and their name.  If a candidate
// has a status CAND_DROPPED their count and percentage is printed as
// a "-" to indicate their dropped status. All other candidates have
// their count printed as numbers.
//
// The width format for each column is as follows
// - NUM: integer, 3 wide, right aligned
// - COUNT: integer, 5 wide, right aligned
// - %PERC: floating point, 5 wide, 1 decimal place, right aligned
// - S: status of the candidate, one of A, M, D for ACTIVE, MINVOTES, DROPPED
// - NAME: string, left aligned
// The format specifiers of printf() are used to format these fields.
//
// If there are 0 total votes, this function has undefined behavior
// and may print random garbage. This situation will not be tested for
// any particular behavior.
//
// MAKEUP CREDIT: If there are more than 0 invalid votes, also prints
// the count of the invalid votes like the following:
//
// Invalid vote count: 5
//
// If there are no valid votes, this function prints the percentage
// for each candidate as 0.0% which is a special case.

void tally_set_minvote_candidates(tally_t *tally){
 if (tally == NULL || tally->candidate_count == 0) {
        return;
    }

    int min_votes = -1;

    for (int i = 0; i < tally->candidate_count; i++) {
        if (tally->candidate_status[i] != CAND_DROPPED) {
            if (min_votes == -1 || tally->candidate_vote_counts[i] < min_votes) {
                min_votes = tally->candidate_vote_counts[i];
            }
        }
    }

    if (min_votes == -1) {
        if (LOG_LEVEL >= LOG_MINVOTE) {
            printf("LOG: No MIN VOTE count found\n");
        }
        return;
    }

    if (LOG_LEVEL >= LOG_MINVOTE) {
        printf("LOG: MIN VOTE count is %d\n", min_votes);
    }

    for (int i = 0; i < tally->candidate_count; i++) {
        if (tally->candidate_status[i] == CAND_ACTIVE && tally->candidate_vote_counts[i] == min_votes) {
            tally->candidate_status[i] = CAND_MINVOTES;
            if (LOG_LEVEL >= LOG_MINVOTE) {
                printf("LOG: MIN VOTE COUNT for candidate %d: %s\n", i, tally->candidate_names[i]);
            }
        }
    }
}
// PROBLEM 1: Scans the vote counts of candidates and sets the status
// of candidates with the minimum votes to CAND_MINVOTES excluding
// those with status CAND_DROPPED. All candidates with the minumum
// number of votes have their status set to CAND_MINVOTES.
//
// EXAMPLE:
//
// tally_t t = {
//   .candidate_count = 4;
//   .candidate_names = {"Francis",   "Claire",      "Heather",    "Viktor"},
//   .candidate_status= {CAND_DROPPED, CAND_ACTIVE,   CAND_ACTIVE,  CAND_ACTIVE},
//   .candidate_vote_counts = {0,      4,             2,            2}
// }
// tally_set_minvote_candidates(&t);
// t is now {
//   .candidate_count = 4;
//   .candidate_names = {"Francis",   "Claire",      "Heather",     "Viktor"},
//   .candidate_status= {CAND_DROPPED, CAND_ACTIVE,   CAND_MINVOTES, CAND_MINVOTES},
//   .candidate_vote_counts = {0,      4,             2,             2}
// }
//
// Two candidates have changed status to CAND_MINVOTES but the 0th
// candidate who has status CAND_DROPPED is ignored.
//
// LOGGING: if the LOG_LEVEL is >= LOG_MINVOTE, this function will
// print the following messages to standard out while running.
//
// "LOG: No MIN VOTE count found" : printed when the candidate count
// is 0 or all candidates have status CAND_DROPPED.
//
// "LOG: MIN VOTE count is XX" : printed after the minimum vote count is
// determined with XX substituted for the actual minimum vote count.
//
// "LOG: MIN VOTE count for candidate YY: ZZ" : printed for each
// candidate whose status is changed to CAND_MINVOTES with YY and ZZ
// as the candidate index and name.

int tally_condition(tally_t *tally){
   if (tally == NULL) {
        return TALLY_ERROR;
    }

    int active_count = 0;
    int minvote_count = 0;

    for (int i = 0; i < tally->candidate_count; i++) {
        switch (tally->candidate_status[i]) {
            case CAND_ACTIVE:
                active_count++;
                break;
            case CAND_MINVOTES:
                minvote_count++;
                break;
            case CAND_DROPPED:
                break;
            default:
                return TALLY_ERROR;
        }
    }

    if (active_count == 1) {
        return TALLY_WINNER;
    } else if (active_count > 1) {
        return TALLY_CONTINUE;
    } else if (active_count == 0 && minvote_count > 1) {
        return TALLY_TIE;
    }

    return TALLY_ERROR;
}
// PROBLEM 1: Determine the current condition of the given tally which
// is one of {TALLY_ERROR TALLY_WINNER TALLY_TIE TALLY_CONTINUE}. The
// condition is determined by counting the status of candidates and
// returning a value based on the following circumstances.
//
// - If any candidate has a status outher than CAND_ACTIVE,
//   CAND_MINVOTES, CAND_DROPPED, returns TALLY_ERROR as something has
//   gone wrong tabulations.
// - If there is only 1 ACTIVE candidate, returns TALLY_WINNER as
//   the election has determined a winner
// - If there are 2 or more ACTIVE candidates, returns TALLY_CONTINUE as
//   additional rounds are needed to determine winner
// - If there are 0 ACTIVE candidates and 2 or more MINVOTE candidates,
//   returns TALLY_TIE as the election has ended with a Multiway Tie
// - Returns TALLY_ERROR in all other cases as something has gone wrong
//   in the tabulation (e.g. all candidates dropped, a single MINVOTE
//   candidate, some other bad state).

////////////////////////////////////////////////////////////////////////////////
// PROBLEM 2 Functions

vote_t *vote_make_empty(){
    // Allocate memory for vote_t structure
    vote_t *new_vote = (vote_t *)malloc(sizeof(vote_t));
    if (new_vote == NULL) {
        return NULL; 
    }

    // Initialize vote fields
    new_vote->id = -1;
    new_vote->pos = -1;
    new_vote->next = NULL;
    for (int i = 0; i < MAX_CANDIDATES; i++) {
        new_vote->candidate_order[i] = NO_CANDIDATE;
    }

    return new_vote; 
}
// PROBLEM 2: Allocates a vote on the heap using malloc() and
// intitializes its id/pos fields to be -1, all of the entries in
// its candidate_order[] array to be NO_CANDIDATE, and the next field
// to NULL. Returns a pointer to that vote.

void tally_free(tally_t *tally){
if (tally == NULL) {
        return;
    }

    // Free...
    for (int i = 0; i < tally->candidate_count; i++) {
        vote_t *current = tally->candidate_votes[i];
        while (current != NULL) {
            vote_t *next = current->next;
            free(current);
            current = next;
        }
    }

    // Free tally
    free(tally);
}
// PROBLEM 2: De-allocates a tally and all its linked votes from the
// heap using free(). The entirety of the candidate_votes[] array is
// traversed and each list of votes in it is free()'d by iterating
// through each list and free()'ing each vote. Ends by free()'ing the
// tally itself.
//
// MAKEUP CREDIT: In addition to the candidate vote lists, also
// de-allocates the invalid vote list.

void tally_add_vote(tally_t *tally, vote_t *vote){
 if (tally == NULL || vote == NULL) {
        return;
    }

    int candidate_index = vote->candidate_order[vote->pos];

    // Prepend vote list
    vote->next = tally->candidate_votes[candidate_index];
    tally->candidate_votes[candidate_index] = vote;

    // Increment vote count
    tally->candidate_vote_counts[candidate_index]++;
}
// PROBLEM 2: Add the given vote to the given tally. The vote is
// assigned to candidate indicated by the vote->pos field and
// vote->candidate_order[] array.  The vote is prepended (added to the
// front) of the associated candidates list of votes and their vote
// count is incremented. This function is primarily used when
// initially populating a tally while other functions like
// tally_transfer_first_vote() are used when calculating elections.
//
// MAKEUP CREDIT: Votes whose preference is NO_CANDIDATE are prepended
// to the invalid_votes list with the invalid_vote_count incrementing.

void tally_print_votes(tally_t *tally){
if (tally == NULL) {
        return;
    }

    for (int i = 0; i < tally->candidate_count; i++) {
        printf("VOTES FOR CANDIDATE %d: %s\n", i, tally->candidate_names[i]);

        int vote_count = 0;
        vote_t *current = tally->candidate_votes[i];
        while (current != NULL) {
            printf("  ");
            vote_print(current);
            printf("\n");
            current = current->next;
            vote_count++;
        }

        printf("%d votes total\n", vote_count);
    }
}
// PROBLEM 2: Prints out the votes for each candidate in the tally
// which produces output like the following:
//
// VOTES FOR CANDIDATE 0: Andy
//   #0005:<0> 1  3  2  4
//   #0004:<0> 1  2  3  4
// 2 votes total
// VOTES FOR CANDIDATE 1: Bethany
// 0 votes total
// VOTES FOR CANDIDATE 2: Carl
//   #0002: 3 <2> 4  1  0
//   #0003:<2> 1  0  3  4
//   #0001:<2> 0  1  3  4
// 3 votes total
// ...
//
// - Each set of votes is preceded by the headline
//   "VOTES FOR CANDIDATE XX: YY"
//   with XX and YY as the candidate index and name.
// - Each candidate vote is printed starting with 2 spaces, then via a
//   call to vote_print(); then a newline. The list of votes for a
//   particular candidate is printed via iteration through the list
//   following the `next` field of the vote_t struct.
// - Each candidate vote list is ended with a line reading
//   "ZZ votes total"
//   with ZZ replaced by the count of votes for that candidate.
//
// MAKEUP CREDIT: If there are any invalide votes, an additional headline
// "INVALID VOTES"
// is printed followed by a listing of invalid votes in the same
// format as above and ending with a line showing the total invalid
// votes.

void tally_transfer_first_vote(tally_t *tally, int candidate_index){
     if (tally == NULL || candidate_index >= tally->candidate_count || tally->candidate_votes[candidate_index] == NULL) {
        return;
    }

    // Get the first vote for the candidate
    vote_t *vote_to_transfer = tally->candidate_votes[candidate_index];

    tally->candidate_votes[candidate_index] = vote_to_transfer->next;
    tally->candidate_vote_counts[candidate_index]--;

    // Get the next preferred candidate
    vote_to_transfer->pos++; 
    int next_candidate = vote_next_candidate(vote_to_transfer, tally->candidate_status);

    if (next_candidate == NO_CANDIDATE) {
        vote_to_transfer->next = tally->candidate_votes[candidate_index];
        tally->candidate_votes[candidate_index] = vote_to_transfer;
        tally->candidate_vote_counts[candidate_index]++;
    } else {
        // Add the vote to the next candidate's list
        vote_to_transfer->next = tally->candidate_votes[next_candidate];
        tally->candidate_votes[next_candidate] = vote_to_transfer;
        tally->candidate_vote_counts[next_candidate]++;

        // Log the vote transfer
        if (LOG_LEVEL >= LOG_VOTE_TRANSFERS) {
            printf("LOG: Transferred Vote ");
            vote_print(vote_to_transfer);
            printf(" from %d %s to %d %s\n",
                   candidate_index, tally->candidate_names[candidate_index],
                   next_candidate, tally->candidate_names[next_candidate]);
        }
    }
}
// PROBLEM 2: Transfer the first vote for the candidate at
// `candidate_index` to the next candidate indicated on the vote. This
// is usually done when the indicated candidate is being dropped from
// the election and their votes are being re-assigned to others.
//
// # COUNT NAME    VOTES
// 0     4 Francis #0008: 3 <0> 2  1 #0009:<0> 1  2  3 #0005:<0> 1  2  3 #0001:<0> 3  2  1
// 1     2 Claire  #0004:<1> 0  2  3 #0002:<1> 0  2  3
// 2     4 Heather #0010:<2> 0  1  3 #0007:<2> 0  1  3 #0006:<2> 1  0  3 #0003:<2> 1  0  3
// 3     0 Viktor
//
// transfer_first_vote(tally, 1);  // Claire's first vote to Francis
//
// # COUNT NAME    VOTES
// 0     5 Francis #0004: 1 <0> 2  3 #0008: 3 <0> 2  1 #0009:<0> 1  2  3 #0005:<0> 1  2  3 #0001:<0> 3  2  1
// 1     1 Claire  #0002:<1> 0  2  3
// 2     4 Heather #0010:<2> 0  1  3 #0007:<2> 0  1  3 #0006:<2> 1  0  3 #0003:<2> 1  0  3
// 3     0 Viktor
//
// Note that vote #0002 moves from the front of Claire's list to the
// front of Francis's list.  The `candidate_vote_count[]` array is
// also updated. The function vote_next_candidate(vote) is used to
// alter the vote to reflect the voters next preferred candidate and
// that function's return value is used to determine the destination
// candidate for the transfer. If the candidate at `candidate_index`
// has no votes (vote list is empty), this function does nothing and
// immediately returns.
//
// LOGGING: if LOG_LEVEL >= LOG_VOTE_TRANSFERS then the following message
// is printed:
// "LOG: Transferred Vote #0002: 1 <0> 2  3  from 1 Claire to 0 Francis"
// where the details are adapted to the actual data. Make use of the
// vote_print() function to show the vote.
//
// MAKEUP CREDIT: Votes which return a NO_CANDIDATE result from
// vote_next_candidate() are moved to the invalid_votes list with a
// message to that effect printed:
// "Transferred Vote #0002: 1 <0> 2  3  from 1 Claire to Invalid Votes"

void tally_drop_minvote_candidates(tally_t *tally){
if (tally == NULL) {
        return;
    }

    for (int i = 0; i < tally->candidate_count; i++) {
        if (tally->candidate_status[i] == CAND_MINVOTES) {
            // Transfer all votes for this candidate
            while (tally->candidate_votes[i] != NULL) {
                tally_transfer_first_vote(tally, i);
            }

            // Mark the candidate as dropped
            tally->candidate_status[i] = CAND_DROPPED;

            // Log the candidate drop
            if (LOG_LEVEL >= LOG_DROP_MINVOTES) {
                printf("LOG: Dropped Candidate %d: %s\n", i, tally->candidate_names[i]);
            }
        }
    }
}
// PROBLEM 2: All candidates with the status CAND_MINVOTES have their
// votes transferred to other candidates via repeated calls to
// tally_transfer_first_vote(). Those with status CAND_MINVOTE are
// changed to have CAND_DROPPED to indicate they are no longer part of
// the election.
//
// LOGGING: If LOG_LEVEL >= LOG_DROP_MINVOTES, prints the following
// for each MINVOTE candidate that is DROPPED:
// "LOG: Dropped Candidate XX: YY"
// with XX and YY as the candidate index and name respectively.

void tally_election(tally_t *tally){
    if (tally == NULL) {
        return;
    }

    int round = 1;
    int condition;

    while (1) {
        printf("=== ROUND %d ===\n", round);

        tally_drop_minvote_candidates(tally);

        tally_print_table(tally);

        if (LOG_LEVEL >= LOG_SHOWVOTES) {
            tally_print_votes(tally);
        }

        tally_set_minvote_candidates(tally);

        condition = tally_condition(tally);

        if (condition != TALLY_CONTINUE) {
            break;
        }

        round++;
    }

    // Print final result based on the tally condition
    if (condition == TALLY_WINNER) {
        for (int i = 0; i < tally->candidate_count; i++) {
            if (tally->candidate_status[i] == CAND_ACTIVE) {
                printf("Winner: %s (candidate %d)\n", tally->candidate_names[i], i);
                return;
            }
        }
    } else if (condition == TALLY_TIE) {
        printf("Multiway Tie Between:\n");
        for (int i = 0; i < tally->candidate_count; i++) {
            if (tally->candidate_status[i] == CAND_MINVOTES) {
                printf("%s (candidate %d)\n", tally->candidate_names[i], i);
            }
        }
    } else if (condition == TALLY_ERROR) {
        printf("Something is rotten in the state of Denmark\n");
    }
}
// PROBLEM 2: Executes an election on the given tally.  Repeatedly
// performs the following operations.
//
// - Prints a headline "=== ROUND NN ===" with NN starting at 1 and
//   incrementing each round of the election
// - Drops the minimum vote candidates from the tally; in the first round
//   there will be no MINVOTE candidates but subsequent rounds may have 1
//   or more
// - Prints a table of the current tally state
// - If the LOG_LEVEL >= LOG_SHOWVOTES or more, print all votes for all
//   candidates using an appropriate function; otherwise don't print
//   anything
// - Determine the MINVOTE candidate(s) and cycle to the next round
// Rounds continue while the Condition of the tally is
// TALLY_CONTINUE. When the election ends, one of the following messages
// is printed.
// - If a WINNER was found, print
//   "Winner: XX (candidate YY)"
//   with XX as the candidate name and YY as their index
// - If a TIE resulted, print each candidate that tied as in
//   "Multiway Tie Between:"
//   "AA (candidate XX)"
//   "BB (candidate YY)"
//   "CC (candidate ZZ)"
//   with AA,BB,CC as the candidate names and XX,YY,ZZ their indices.
// - If an ERROR in the election occurred, print
//   "Something is rotten in the state of Denmark"
//
// To print out winners / tie members, this function will iterate
// through the candidate_status[] array to examine the status of each
// candidate. A single winner will be the only CAND_ACTIVE candidate
// while members of a TIE will each have the state CAND_MINVOTES with no
// ACTIVE candidate.
//
// At LOG_LEVEL=0, the output for this function looks like the
// following:
// === ROUND 1 ===
// NUM COUNT %PERC S NAME
//   0     4  33.3 A Francis
//   1     2  16.7 A Claire
//   2     5  41.7 A Heather
//   3     1   8.3 A Viktor
// === ROUND 2 ===
// NUM COUNT %PERC S NAME
//   0     5  41.7 A Francis
//   1     2  16.7 A Claire
//   2     5  41.7 A Heather
//   3     -     - D Viktor
// === ROUND 3 ===
// NUM COUNT %PERC S NAME
//   0     7  58.3 A Francis
//   1     -     - D Claire
//   2     5  41.7 A Heather
//   3     -     - D Viktor
// Winner: Francis (candidate 0)
//

////////////////////////////////////////////////////////////////////////////////
// PROBLEM 3 FUNCTIONS

tally_t *tally_from_file(char *fname){

    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        printf("ERROR: couldn't open file '%s'\n", fname);
        return NULL;
    }

    if (LOG_LEVEL >= LOG_FILEIO) {
        printf("LOG: File '%s' opened\n", fname);
    }

    // Allocate memory for the tally
    tally_t *tally = (tally_t *)malloc(sizeof(tally_t));
    if (tally == NULL) {
        fclose(file);
        return NULL;
    }

    // Initialize tally fields
    tally->candidate_count = 0;
    tally->invalid_vote_count = 0;
    tally->invalid_votes = NULL;
    for (int i = 0; i < MAX_CANDIDATES; i++) {
        tally->candidate_vote_counts[i] = 0;
        tally->candidate_status[i] = CAND_DROPPED; 
        tally->candidate_votes[i] = NULL; 
        memset(tally->candidate_names[i], 0, MAX_NAME);
    }

    // Read the number of candidates
    if (fscanf(file, "%d", &(tally->candidate_count)) != 1) {
        printf("ERROR: failed to read number of candidates\n");
        fclose(file);
        free(tally);
        return NULL;
    }

    if (LOG_LEVEL >= LOG_FILEIO) {
        // Log message with the typo to match expected output
        printf("LOG: File '%s' has %d candidtes\n", fname, tally->candidate_count);
    }

    // Read candidate names
    for (int i = 0; i < tally->candidate_count; i++) {
        if (fscanf(file, "%s", tally->candidate_names[i]) != 1) {
            printf("ERROR: failed to read candidate names\n");
            fclose(file);
            tally_free(tally);
            return NULL;
        }
        tally->candidate_status[i] = CAND_ACTIVE;
        if (LOG_LEVEL >= LOG_FILEIO) {
            printf("LOG: File '%s' candidate %d is %s\n", fname, i, tally->candidate_names[i]);
        }
    }

    // Read votes until the end of the file
    int vote_id = 1;
    while (1) {
        vote_t *vote = vote_make_empty();
        if (vote == NULL) {
            printf("ERROR: memory allocation failed for vote\n");
            fclose(file);
            tally_free(tally);
            return NULL;
        }

        vote->id = vote_id++;
        int status;
        for (int i = 0; i < tally->candidate_count; i++) {
            status = fscanf(file, "%d", &(vote->candidate_order[i]));
            if (status != 1) {
                free(vote);  
                break;
            }
        }

        if (status != 1) { // End of file 
            break;
        }

        // Set initial preference
        vote->pos = 0;

        // Log the vote read with correct format
        if (LOG_LEVEL >= LOG_FILEIO) {
            printf("LOG: File '%s' vote #%04d:<%d> ", fname, vote->id, vote->candidate_order[0]);
            for (int i = 1; i < tally->candidate_count; i++) {
                printf("%d ", vote->candidate_order[i]);
            }
            printf("\n");
        }

        // Add vote
        tally_add_vote(tally, vote);
    }

    // Log that the end of file is reached
    if (LOG_LEVEL >= LOG_FILEIO) {
        printf("LOG: File '%s' end of file reached\n", fname);
    }

    // Close
    fclose(file);
    return tally;
}
// PROBLEM 3: Opens the given `fname` and reads its contents to create
// a tally with votes assigned to candidates.  The format of the input
// file is as follows (# denotes comments that will not appear in the
// actual files)
//
// EXAMPLE 1: 4 candidates, 6 votes
// 4                               # first token in number of candidates
// Francis Claire Heather Viktor   # names of the 4 candidate
// 0 3 2 1                         # vote #0001 with preference of 4 candidates
// 1 0 2 3                         # vote #0002 with preference of 4 candidates
// 2 1 0 3                         # etc.
// 2 1 0 3
// 1 0 2 3
// 0 2 1 3
//
// EXAMPLE 2: 5 candidates, 7 votes
// 5                              # first token in number of candidates
// Al Bo Ce Di Ed                 # names of the 5 candidate
// 2 0 1 3 4                      # vote #0001 preference of 5 candidates
// 3 2 4 1 0                      # etc.
// 2 1 0 3 4
// 0 1 2 3 4
// 0 1 3 2 4
// 3 2 4 1 0
// 2 1 0 3 4
//
// Other examples are present in the "data/" directory.
//
// This function heap-allocates a tally_t struct then begins reading
// information from the file into the fields of that struct starting
// with the number of candidates and their names.  A loop is then used
// to iterate reading votes until the End of the File (EOF) is
// reached.  On determining that there is a vote to read, an empty
// vote_t is allocated using vote_make_empty() and the order
// preference of candidates is read into the vote along with
// initializing its pos and id fields. It is then added to the tally
// via tally_add_vote() before iterating to try to read another vote.
//
// This function makes heavy use of fscanf() to read data and checks
// the return value of fscanf() at times to determine if the end of a
// file has been reached. On reaching the end of the input, the file
// is closed and the completed tally is returned
//
// ERROR CASES: Near the beginning of its operation, this function
// checks that the specified file is opened successfully. If not, it
// prints the message
// "ERROR: couldn't open file 'XX'"
// with XX as the filename. NULL is returned in this case.
//
// Aside from failure to open a file, this function assumes that the
// data is formatted correctly and does no other error handling.
// - The first token is NCAND, the number of candidates
// - The next tokens are NCAND strings which are the candidate names
// - Each subsequent vote has exactly NCAND integers
// Bad input data that does not follow the above conventions will
// cause this function to have unpredictable behavior that is not
// tested.
//
// LOGGING: If LOG_LEVEL >= LOG_FILEIO, this function prints the
// following messages which show the progress of the
// function. Substitute XX and CC and such with the actual data read.
//
// "LOG: File 'XX' opened" : when the file is successfully opened
// "LOG: File 'XX' has CC candidtes" : after reading the number of candidates
// "LOG: File 'XX' candidate CC is YY" : after reading a candidate name
// "LOG: File 'XX' vote #0123 <0> 2 3 1" : after reading a comple vote
// "LOG: File 'XX' end of file reached" : on reaching the end of the file
//
// MAKEUP CREDIT: Handles readin NO_CANDIDATE (-1) entries in the
// candidate order. If the first preference in a vote is -1, it is
// immediately placed in the Invalid Vote list

int main(int argc, char *argv[]); // this function in rcv_main.c
// PROBLEM 3: main() in rcv_main.c
