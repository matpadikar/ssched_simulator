#ifndef SERENA_HPP
#define SERENA_HPP
// This file is based on Long Gong's work
#include <vector>
#include <cassert>

#define LWS_UNMATCHED -1
#define LWS_MATCHED 1

namespace saber{
int matching_size(const std::vector<int>& S){
    int len = S.size();
    std::vector<int> visited(S.size(), LWS_UNMATCHED);
    int result = 0;

    for(int i = 0;i < len;++ i){
        if (S[i] >= 0 && S[i] < len) {
            if (visited[S[i]] == LWS_MATCHED){
                return -1; // not a matching
            }
            ++result;
            visited[S[i]] = LWS_MATCHED;
        }
    }

    return result;
}

bool is_matching(const std::vector<int>& S)
{
    int len = S.size();
    std::vector<int> visited(S.size(), LWS_UNMATCHED);
    bool result = true;

    for(int i = 0;i < len;++ i)
        if (S[i] >= 0 && S[i] < len) visited[S[i]] = LWS_MATCHED;
        else { result = false; break; }

    for (int i = 0;i < len;++ i)
        if (visited[i] != LWS_MATCHED) { result = false; break; }
    return result;
}

void fix_matching(std::vector<int>& S)
{
    int len = S.size();
    std::vector<int> is_matched(len, LWS_UNMATCHED);
    std::vector<int> unmatched_in;
    std::vector<int> unmatched_out;

    int i, j, k;
    // int unmatched_counter = 0;

    /*! find all unmatched inputs & mark all matched inputs */
    for (i = 0;i < len;++ i)
        if (S[i] != LWS_UNMATCHED) is_matched[S[i]] = LWS_MATCHED; /*! mark matched output */
        else unmatched_in.push_back(i); /*! record unmatched inputs */

    /*! find all unmatched outputs */
    for (k = 0;k < len;++ k)
        if (is_matched[k] == LWS_UNMATCHED) unmatched_out.push_back(k);

    assert(unmatched_in.size() == unmatched_out.size());

    /*! matched all the unmatched inputs and outputs (round-robin) */
    for (j = 0;j < unmatched_in.size();++ j) {
        i = unmatched_in[j];
        k = unmatched_out[j];
        S[i] = k;
    }
}

// merge S1 and S2 to S
void merge(
    const std::vector<int>& S1, const std::vector<int>& S2, 
    std::vector<int>& S, const std::vector<std::vector<int> >& weights) {
    assert(is_matching(S1) && is_matching(S2));
    assert(S1.size() == S2.size());
    assert(S1.size() == S.size());
    //S.resize(S1.size());

    int N = S1.size();
    int i, k, j, c, w, ibk;

    std::vector<int> S2_reverse(N, 0); /* reverse matching of S2 */

    for (i = 0;i < N;++ i) S2_reverse[S2[i]] = i;

    int unmatched_inputs = N;
    int current = 0;

    std::vector<int> input_matched_counter(N, -1);
    std::vector<int> selected_matching_id(N, 0);

    c = 0;/* cycle counter */
    while (unmatched_inputs > 0) {
        for (i = 0;i < N;++ i) if (input_matched_counter[i] == -1) break; /*! find the first unmatched input */
        ibk = i;
        input_matched_counter[ibk] = c;
        -- unmatched_inputs;
        current = S1[i]; /* corresponding output */
        w = weights[i][current];
        for (j = 0;j < 2 * N;++ j) {
            k = current;
            current = S2_reverse[k];/* new input port */
            w -= weights[current][k];
            if (current == ibk) break; /* back to i */
            else {
                i = current;
                input_matched_counter[i] = c; /* add label */
                -- unmatched_inputs;
                current = S1[i];/* output port */
                w += weights[i][current];/* update weight difference */
            }
        }
        if (w < 0) selected_matching_id[c] = 1;
        ++ c;
    }

    S.resize(S1.size());
    for (i = 0;i < N;++ i) {
        c = input_matched_counter[i]; /* in which iteration this input is visited */
        assert(c != -1);
        S[i] = selected_matching_id[c] == 0? S1[i] : S2[i];
    }

    assert(is_matching(S)); /*! merged match should be a full matching */
}
} // namespace saber
#endif